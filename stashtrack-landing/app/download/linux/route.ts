import { NextResponse } from 'next/server'
import { getLatestInstallerRelease } from '../../../lib/githubRelease'

export const dynamic = 'force-dynamic'

export async function GET() {
  const release = await getLatestInstallerRelease()
  const download = release.platforms.linux

  return NextResponse.redirect(
    download.available ? download.installerUrl : release.releasePageUrl,
    302,
  )
}
