// Offscreen UI probe: loads the real jsui-vsreact bundle into a RootView with
// stubbed natives, drives it with synthetic mouse events (no OS input), and
// writes PNG snapshots. Lets us verify the drawer/preview/progress flows even
// while the desktop's real mouse is busy elsewhere.
//
//   StashTrackUiProbe <bundlePath> <outDir>

#include <juce_gui_extra/juce_gui_extra.h>
#include <vsreact/vsreact.h>

#include <iostream>

namespace
{
    juce::var makeHistory()
    {
        juce::Array<juce::var> list;
        const char* names[] = { "Late Night Chords.wav", "Vinyl Drum Break.wav",
                                "Analog Bass Stab.wav", "Tape Choir Pad.wav",
                                "Granular Sparkles.wav", "Lo-Fi Keys Loop.wav",
                                "Dusty Rhodes Riff.wav", "Ambient Field Rec.wav" };

        auto now = static_cast<double> (juce::Time::currentTimeMillis());

        for (const auto* name : names)
        {
            auto* entry = new juce::DynamicObject();
            entry->setProperty ("path", juce::String ("C:/fake/") + name);
            entry->setProperty ("name", name);
            entry->setProperty ("addedMs", now);
            now -= 3600000.0;
            list.add (juce::var (entry));
        }

        return juce::var (std::move (list));
    }

    void snapshot (juce::Component& component, const juce::File& file)
    {
        const auto image = component.createComponentSnapshot (component.getLocalBounds());
        juce::PNGImageFormat png;

        file.deleteFile();   // FileOutputStream appends by default
        juce::FileOutputStream stream (file);

        if (stream.openedOk())
            png.writeImageToStream (image, stream);
    }

    void pump (int milliseconds)
    {
        juce::MessageManager::getInstance()->runDispatchLoopUntil (milliseconds);
    }
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    const juce::File bundle (argc > 1 ? juce::String (argv[1]) : juce::String());
    const juce::File outDir (argc > 2 ? juce::String (argv[2]) : juce::String());
    outDir.createDirectory();

    bool previewPlaying = false;

    vsreact::RootOptions options;
    options.bundleFile = bundle;
    options.onNativeCall = [&previewPlaying] (const juce::String& name, const juce::var& args) -> juce::var
    {
        juce::ignoreUnused (args);

        if (name == "getInitialState")
        {
            auto* state = new juce::DynamicObject();
            state->setProperty ("version", "0.8.0-probe");
            state->setProperty ("choice", "Downloads: E:\\Downloads");
            return juce::var (state);
        }

        if (name == "history:get" || name == "history:remove")
            return makeHistory();

        if (name == "history:load")
        {
            auto* result = new juce::DynamicObject();
            result->setProperty ("ok", true);
            result->setProperty ("fileName", args["path"].toString().fromLastOccurrenceOf ("/", false, false));
            return juce::var (result);
        }

        if (name == "preview:toggle")
        {
            previewPlaying = ! previewPlaying;
            auto* result = new juce::DynamicObject();
            result->setProperty ("playing", previewPlaying);
            return juce::var (result);
        }

        return {};
    };

    vsreact::RootView root (std::move (options), {});
    root.setSize (720, 470);

    // Let the splash play out.
    pump (3600);
    snapshot (root, outDir.getChildFile ("p1-main.png"));

    auto& source = juce::Desktop::getInstance().getMainMouseSource();
    const auto mods = juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier);

    const auto makeEvent = [&root, &source, &mods] (juce::Point<float> position)
    {
        const auto now = juce::Time::getCurrentTime();
        return juce::MouseEvent (source, position, mods,
                                 juce::MouseInputSource::defaultPressure,
                                 juce::MouseInputSource::defaultOrientation,
                                 juce::MouseInputSource::defaultRotation,
                                 juce::MouseInputSource::defaultTiltX,
                                 juce::MouseInputSource::defaultTiltY,
                                 &root, &root, now, position, now, 1, false);
    };

    const auto click = [&] (float x, float y)
    {
        root.mouseDown (makeEvent ({ x, y }));
        pump (30);
        root.mouseUp (makeEvent ({ x, y }));
        pump (120);
    };

    // Open the stash drawer (header pill, top right).
    click (657.0f, 33.0f);
    pump (500);
    snapshot (root, outDir.getChildFile ("p2-drawer.png"));

    for (auto* child : root.getChildren())
        if (auto* host = dynamic_cast<vsreact::TextInputHost*> (child))
            std::cout << "textinput host bounds=" << host->getBounds().toString()
                      << " visible=" << (host->isVisible() ? 1 : 0) << std::endl;

    // Wheel-scroll the drawer list.
    juce::MouseWheelDetails wheel;
    wheel.deltaX = 0.0f;
    wheel.deltaY = -0.35f;
    root.mouseWheelMove (makeEvent ({ 560.0f, 300.0f }), wheel);
    root.mouseWheelMove (makeEvent ({ 560.0f, 300.0f }), wheel);
    pump (150);
    snapshot (root, outDir.getChildFile ("p3-drawer-scrolled.png"));

    // Load the first visible entry, then close via the backdrop.
    click (560.0f, 150.0f);
    pump (250);
    click (120.0f, 300.0f);
    pump (450);
    snapshot (root, outDir.getChildFile ("p4-loaded.png"));

    // Toggle preview (PLAY pill on the waveform card header).
    click (470.0f, 265.0f);
    pump (300);
    snapshot (root, outDir.getChildFile ("p5-preview.png"));

    std::cout << "probe snapshots written to " << outDir.getFullPathName() << std::endl;
    return 0;
}
