import { NextResponse } from 'next/server'

export const dynamic = 'force-dynamic'

const FALLBACK_INSTALLER_URL =
  'https://github.com/davad00/StashTrack/releases/download/v0.4/StashTrackv0.4Setup.exe'

type GitHubReleaseAsset = {
  name: string
  browser_download_url: string
}

type GitHubRelease = {
  assets?: GitHubReleaseAsset[]
}

function findWindowsInstallerAsset(release: GitHubRelease) {
  return release.assets?.find((asset) =>
    /^StashTrackv.+Setup\.exe$/i.test(asset.name),
  )
}

export async function GET() {
  try {
    const response = await fetch(
      'https://api.github.com/repos/davad00/StashTrack/releases/latest',
      {
        headers: {
          Accept: 'application/vnd.github+json',
          'User-Agent': 'stashtrack.n9records.com',
        },
        cache: 'no-store',
      },
    )

    if (!response.ok) {
      return NextResponse.redirect(FALLBACK_INSTALLER_URL, 302)
    }

    const release = (await response.json()) as GitHubRelease
    const installer = findWindowsInstallerAsset(release)

    return NextResponse.redirect(
      installer?.browser_download_url ?? FALLBACK_INSTALLER_URL,
      302,
    )
  } catch {
    return NextResponse.redirect(FALLBACK_INSTALLER_URL, 302)
  }
}
