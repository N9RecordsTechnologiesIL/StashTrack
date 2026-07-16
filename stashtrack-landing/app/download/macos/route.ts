import { NextResponse } from 'next/server'
import { getLatestInstallerRelease } from '../../../lib/githubRelease'

export const dynamic = 'force-dynamic'

export async function GET() {
  const release = await getLatestInstallerRelease()
  const download = release.platforms.macos

  // Older releases predate macOS builds — land on the release page instead of
  // a 404 asset.
  return NextResponse.redirect(
    download.available ? download.installerUrl : release.releasePageUrl,
    302,
  )
}
