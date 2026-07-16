const GITHUB_REPO = 'N9RecordsTechnologiesIL/StashTrack'
const LATEST_RELEASE_API_URL = `https://api.github.com/repos/${GITHUB_REPO}/releases/latest`
const LATEST_RELEASE_PAGE_URL = `https://github.com/${GITHUB_REPO}/releases/latest`
const STABLE_INSTALLER_ASSET_NAME = 'StashTrackSetup.exe'

export type Platform = 'windows' | 'macos' | 'linux'

// Stable asset names are re-uploaded on every release, so the versionless
// latest-download URL always resolves; versioned names are derived from the
// tag as a fallback when the JSON API is unavailable.
const PLATFORMS: Record<
  Platform,
  { stableAssetName: string; versionedPattern: RegExp; versionedNameForTag: (tag: string) => string }
> = {
  windows: {
    stableAssetName: STABLE_INSTALLER_ASSET_NAME,
    versionedPattern: /^StashTrackv.+Setup\.exe$/i,
    versionedNameForTag: (tag) => `StashTrack${tag}Setup.exe`,
  },
  macos: {
    stableAssetName: 'StashTrack-macOS.pkg',
    versionedPattern: /^StashTrackv.+-macOS\.pkg$/i,
    versionedNameForTag: (tag) => `StashTrack${tag}-macOS.pkg`,
  },
  linux: {
    stableAssetName: 'StashTrack-linux-x86_64.tar.gz',
    versionedPattern: /^StashTrackv.+-linux-x86_64\.tar\.gz$/i,
    versionedNameForTag: (tag) => `StashTrack${tag}-linux-x86_64.tar.gz`,
  },
}

export const FALLBACK_VERSION_TAG = 'latest'
export const FALLBACK_INSTALLER_URL =
  `https://github.com/${GITHUB_REPO}/releases/latest/download/${STABLE_INSTALLER_ASSET_NAME}`

function stableDownloadUrl(platform: Platform) {
  return `https://github.com/${GITHUB_REPO}/releases/latest/download/${PLATFORMS[platform].stableAssetName}`
}

type GitHubReleaseAsset = {
  name: string
  browser_download_url: string
}

type GitHubRelease = {
  tag_name?: string
  html_url?: string
  assets?: GitHubReleaseAsset[]
}

export type PlatformDownload = {
  installerUrl: string
  /** false when the release carries no asset for this platform (older tags). */
  available: boolean
}

export type LatestInstallerRelease = {
  versionTag: string
  releasePageUrl: string
  /** The Windows installer URL (kept for existing consumers). */
  installerUrl: string
  platforms: Record<Platform, PlatformDownload>
  fallback: boolean
}

function findPlatformAsset(release: GitHubRelease, platform: Platform) {
  const spec = PLATFORMS[platform]
  return (
    release.assets?.find(
      (asset) => asset.name.toLowerCase() === spec.stableAssetName.toLowerCase(),
    ) ?? release.assets?.find((asset) => spec.versionedPattern.test(asset.name))
  )
}

function releasePageUrlForTag(versionTag: string) {
  return `https://github.com/${GITHUB_REPO}/releases/tag/${versionTag}`
}

function versionedInstallerUrlForTag(versionTag: string, platform: Platform = 'windows') {
  return `https://github.com/${GITHUB_REPO}/releases/download/${versionTag}/${PLATFORMS[platform].versionedNameForTag(versionTag)}`
}

function tagFromLatestRedirect(location: string | null) {
  const match = location?.match(/\/releases\/tag\/([^/?#]+)/)
  return match?.[1] ? decodeURIComponent(match[1]) : undefined
}

async function getLatestTagFromGitHubRedirect() {
  const response = await fetch(LATEST_RELEASE_PAGE_URL, {
    method: 'GET',
    redirect: 'manual',
    cache: 'no-store',
    headers: {
      'User-Agent': 'stashtrack.n9records.com',
    },
  })

  return tagFromLatestRedirect(response.headers.get('location')) ?? tagFromLatestRedirect(response.url)
}

function platformsFromRelease(release: GitHubRelease): Record<Platform, PlatformDownload> {
  const result = {} as Record<Platform, PlatformDownload>

  for (const platform of Object.keys(PLATFORMS) as Platform[]) {
    const asset = findPlatformAsset(release, platform)
    result[platform] = asset?.browser_download_url
      ? { installerUrl: asset.browser_download_url, available: true }
      : { installerUrl: stableDownloadUrl(platform), available: false }
  }

  return result
}

function platformsForTag(versionTag: string): Record<Platform, PlatformDownload> {
  const result = {} as Record<Platform, PlatformDownload>

  for (const platform of Object.keys(PLATFORMS) as Platform[]) {
    result[platform] = {
      installerUrl: versionedInstallerUrlForTag(versionTag, platform),
      available: true,
    }
  }

  return result
}

function fallbackPlatforms(): Record<Platform, PlatformDownload> {
  const result = {} as Record<Platform, PlatformDownload>

  for (const platform of Object.keys(PLATFORMS) as Platform[]) {
    result[platform] = { installerUrl: stableDownloadUrl(platform), available: true }
  }

  return result
}

export async function getLatestInstallerRelease(): Promise<LatestInstallerRelease> {
  try {
    const response = await fetch(LATEST_RELEASE_API_URL, {
      headers: {
        Accept: 'application/vnd.github+json',
        'User-Agent': 'stashtrack.n9records.com',
      },
      cache: 'no-store',
    })

    if (!response.ok) {
      throw new Error(`GitHub latest release returned ${response.status}`)
    }

    const release = (await response.json()) as GitHubRelease
    const platforms = platformsFromRelease(release)

    if (!release.tag_name || !platforms.windows.available) {
      throw new Error('Latest release is missing a StashTrack setup asset')
    }

    return {
      versionTag: release.tag_name,
      releasePageUrl: release.html_url ?? releasePageUrlForTag(release.tag_name),
      installerUrl: platforms.windows.installerUrl,
      platforms,
      fallback: false,
    }
  } catch {
    const latestTag = await getLatestTagFromGitHubRedirect().catch(() => undefined)

    if (latestTag) {
      return {
        versionTag: latestTag,
        releasePageUrl: releasePageUrlForTag(latestTag),
        installerUrl: versionedInstallerUrlForTag(latestTag),
        platforms: platformsForTag(latestTag),
        fallback: false,
      }
    }

    return {
      versionTag: FALLBACK_VERSION_TAG,
      releasePageUrl: LATEST_RELEASE_PAGE_URL,
      installerUrl: FALLBACK_INSTALLER_URL,
      platforms: fallbackPlatforms(),
      fallback: true,
    }
  }
}
