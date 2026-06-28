import { NextResponse } from 'next/server'
import { getLatestInstallerRelease } from '../../../lib/githubRelease'

export const dynamic = 'force-dynamic'

export async function GET() {
  const release = await getLatestInstallerRelease()

  return NextResponse.json({
    versionTag: release.versionTag,
    releasePageUrl: release.releasePageUrl,
    installerUrl: release.installerUrl,
    downloadUrl: '/download/windows',
    fallback: release.fallback,
  })
}
