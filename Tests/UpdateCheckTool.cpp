// Manual diagnostic: runs the real update-check pipeline against the live
// GitHub release and prints every intermediate result. Not part of CTest.
//   UpdateCheckTool.exe [currentVersion]

#include <juce_core/juce_core.h>
#include "../Source/UpdateUtils.h"

int main (int argc, char* argv[])
{
    const juce::String currentVersion = argc > 1 ? argv[1] : "0.6.0";

    std::cout << "current version: " << currentVersion << std::endl;

    // Raw fetch with full diagnostics, mirroring createGithubInputStream.
    {
        int statusCode = 0;
        auto stream = juce::URL ("https://api.github.com/repos/davad00/StashTrack/releases/latest")
            .createInputStream (
                juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                    .withConnectionTimeoutMs (15000)
                    .withNumRedirectsToFollow (5)
                    .withExtraHeaders ("Accept: application/vnd.github+json\r\n"
                                       "User-Agent: StashTrack-Updater\r\n")
                    .withStatusCode (&statusCode));

        std::cout << "raw statusCode: " << statusCode
                  << ", stream: " << (stream != nullptr ? "ok" : "null") << std::endl;

        if (stream != nullptr)
        {
            const auto body = stream->readEntireStreamAsString();
            std::cout << "raw body bytes: " << body.getNumBytesAsUTF8() << std::endl;
            std::cout << "raw body head: " << body.substring (0, 200) << std::endl;
        }
    }

    const auto latest = StashTrack::fetchLatestReleaseInfo();
    std::cout << "latest.valid:        " << (latest.valid ? "true" : "false") << std::endl;
    std::cout << "latest.versionTag:   " << latest.versionTag << std::endl;
    std::cout << "latest.installerUrl: " << latest.installerUrl << std::endl;
    std::cout << "latest.releasePage:  " << latest.releasePageUrl << std::endl;

    const auto result = StashTrack::checkForUpdate (currentVersion);
    std::cout << "succeeded:        " << (result.succeeded ? "true" : "false") << std::endl;
    std::cout << "updateAvailable:  " << (result.updateAvailable ? "true" : "false") << std::endl;
    std::cout << "message:          " << result.message << std::endl;

    return 0;
}
