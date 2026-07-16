#pragma once

#include <juce_core/juce_core.h>

namespace StashTrack
{
    struct HistoryEntry
    {
        juce::String path;
        juce::String name;
        juce::int64 addedMs = 0;
    };

    /** The stash: a JSON list of previously downloaded files, newest first,
        capped and pruned to files that still exist on disk. */
    class HistoryStore
    {
    public:
        static constexpr int maxEntries = 50;

        explicit HistoryStore (const juce::File& storageFile);

        /** Default store under the user's app-data folder. */
        static juce::File defaultStorageFile();

        juce::Array<HistoryEntry> load() const;

        /** Adds (or re-fronts) a file and persists. */
        void add (const juce::File& file, juce::int64 nowMs);

        /** Removes one path and persists. */
        void remove (const juce::String& path);

        /** load() minus entries whose file no longer exists (persisted when
            anything was dropped). */
        juce::Array<HistoryEntry> loadExisting();

        juce::var toVar (const juce::Array<HistoryEntry>& entries) const;

    private:
        void save (const juce::Array<HistoryEntry>& entries);

        juce::File storageFile;
    };
}
