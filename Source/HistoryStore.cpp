#include "HistoryStore.h"

namespace StashTrack
{

HistoryStore::HistoryStore (const juce::File& file) : storageFile (file) {}

juce::File HistoryStore::defaultStorageFile()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
        .getChildFile ("StashTrack")
        .getChildFile ("history.json");
}

juce::Array<HistoryEntry> HistoryStore::load() const
{
    juce::Array<HistoryEntry> entries;

    if (! storageFile.existsAsFile())
        return entries;

    const auto parsed = juce::JSON::parse (storageFile.loadFileAsString());

    if (const auto* list = parsed.getArray())
    {
        for (const auto& item : *list)
        {
            HistoryEntry entry;
            entry.path = item["path"].toString();
            entry.name = item["name"].toString();
            entry.addedMs = static_cast<juce::int64> (static_cast<double> (item["addedMs"]));

            if (entry.path.isNotEmpty())
                entries.add (std::move (entry));
        }
    }

    return entries;
}

void HistoryStore::add (const juce::File& file, juce::int64 nowMs)
{
    auto entries = load();

    entries.removeIf ([&file] (const HistoryEntry& entry)
    {
        return entry.path == file.getFullPathName();
    });

    entries.insert (0, { file.getFullPathName(), file.getFileName(), nowMs });

    while (entries.size() > maxEntries)
        entries.removeLast();

    save (entries);
}

void HistoryStore::remove (const juce::String& path)
{
    auto entries = load();
    entries.removeIf ([&path] (const HistoryEntry& entry) { return entry.path == path; });
    save (entries);
}

juce::Array<HistoryEntry> HistoryStore::loadExisting()
{
    auto entries = load();
    const auto before = entries.size();

    entries.removeIf ([] (const HistoryEntry& entry)
    {
        return ! juce::File (entry.path).existsAsFile();
    });

    if (entries.size() != before)
        save (entries);

    return entries;
}

juce::var HistoryStore::toVar (const juce::Array<HistoryEntry>& entries) const
{
    juce::Array<juce::var> list;

    for (const auto& entry : entries)
    {
        auto* object = new juce::DynamicObject();
        object->setProperty ("path", entry.path);
        object->setProperty ("name", entry.name);
        object->setProperty ("addedMs", static_cast<double> (entry.addedMs));
        list.add (juce::var (object));
    }

    return juce::var (std::move (list));
}

void HistoryStore::save (const juce::Array<HistoryEntry>& entries)
{
    storageFile.getParentDirectory().createDirectory();
    storageFile.replaceWithText (juce::JSON::toString (toVar (entries)));
}

}
