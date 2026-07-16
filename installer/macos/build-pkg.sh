#!/bin/sh
# Builds an (unsigned) macOS installer .pkg that places StashTrack.vst3 into
# the system VST3 folder.
#
#   build-pkg.sh <path-to-StashTrack.vst3> <version> <out-dir>
#
# Runtime tools (yt-dlp, ffmpeg) are NOT bundled on macOS — install them with
# `brew install yt-dlp ffmpeg`. The plugin falls back to uvx/PATH lookups.
# The pkg is unsigned/unnotarized: right-click > Open (or allow in System
# Settings > Privacy & Security) on first install.
set -eu

VST3_PATH="$1"
VERSION="$2"
OUT_DIR="$3"

[ -d "$VST3_PATH" ] || { echo "not a .vst3 bundle: $VST3_PATH"; exit 1; }
mkdir -p "$OUT_DIR"

STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

mkdir -p "$STAGE/root"
cp -R "$VST3_PATH" "$STAGE/root/StashTrack.vst3"

PKG="$OUT_DIR/StashTrackv${VERSION}-macOS.pkg"

pkgbuild \
  --root "$STAGE/root" \
  --identifier "com.n9records.stashtrack" \
  --version "$VERSION" \
  --install-location "/Library/Audio/Plug-Ins/VST3" \
  "$PKG"

echo "built $PKG"
