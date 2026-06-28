# StashTrack Landing Page

Live marketing and download site for `stashtrack.n9records.com`.

StashTrack is a JUCE VST3 by N9 Records for FL Studio beatmakers. Paste a URL,
clip the range, download with bundled yt-dlp/Deno/ffmpeg, preview the waveform, and
drag the WAV straight into the playlist.

## Stack

- Next.js 16 App Router
- React 19
- CSS Modules
- Bun for install/build/start
- Render free Node web service

## Local Development

```bash
bun install
bun run dev
```

## Production Check

```bash
bun install --frozen-lockfile
bun run typecheck
bun run build
bun run start -- -H 0.0.0.0 -p 3000
```

## Installer Download

The download buttons point to the local redirect route:

```text
/download/windows
```

That route asks the GitHub Releases API for the latest release and redirects to
the newest `StashTrackv*Setup.exe` asset. Future installer releases do not need
landing-page code changes as long as the release asset keeps that filename
pattern.

The visible version labels call `/api/latest-release`, which uses the same
GitHub latest-release resolver and falls back to the last known installer if
GitHub is temporarily unavailable.

## Render

The repository root includes `render.yaml`. It configures a Render web service
with:

- `runtime: node`
- `plan: free`
- `rootDir: stashtrack-landing`
- `buildCommand: bun install --frozen-lockfile && bun run build`
- `startCommand: bun run start -- -H 0.0.0.0 -p $PORT`
- custom domain: `stashtrack.n9records.com`

## Legal

Only download content you own, have licensed, or have rights to use.

StashTrack is free to use, copy, modify, and share for non-commercial purposes
only. No commercial use or profit is allowed. See the root `LICENSE.md`.

## Support

Questions: [vsts@n9records.com](mailto:vsts@n9records.com)
