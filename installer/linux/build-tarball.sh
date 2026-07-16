#!/bin/sh
# Builds the Linux distribution tarball: the VST3 bundle plus an install
# script that copies it to ~/.vst3.
#
#   build-tarball.sh <path-to-StashTrack.vst3> <version> <out-dir>
#
# Runtime tools are NOT bundled — install yt-dlp and ffmpeg from your package
# manager (they must be on PATH).
set -eu

VST3_PATH="$1"
VERSION="$2"
OUT_DIR="$3"

[ -d "$VST3_PATH" ] || { echo "not a .vst3 bundle: $VST3_PATH"; exit 1; }
mkdir -p "$OUT_DIR"

STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

PKG_DIR="$STAGE/StashTrack-${VERSION}-linux-x86_64"
mkdir -p "$PKG_DIR"
cp -R "$VST3_PATH" "$PKG_DIR/StashTrack.vst3"

cat > "$PKG_DIR/install.sh" <<'EOF'
#!/bin/sh
# Installs StashTrack.vst3 into the per-user VST3 folder.
set -eu
DEST="${HOME}/.vst3"
mkdir -p "$DEST"
rm -rf "$DEST/StashTrack.vst3"
cp -R "$(dirname "$0")/StashTrack.vst3" "$DEST/StashTrack.vst3"
echo "Installed to $DEST/StashTrack.vst3"
echo "Runtime dependencies: yt-dlp and ffmpeg must be on PATH"
echo "  (e.g. sudo apt install ffmpeg && pipx install yt-dlp)"
EOF
chmod +x "$PKG_DIR/install.sh"

cat > "$PKG_DIR/README.txt" <<EOF
StashTrack v${VERSION} — Linux x86_64 VST3

Install:   ./install.sh          (copies to ~/.vst3)
Requires:  yt-dlp and ffmpeg on PATH at runtime.
License:   StashTrack Non-Commercial License (see the GitHub repo).
EOF

TARBALL="$OUT_DIR/StashTrackv${VERSION}-linux-x86_64.tar.gz"
tar -czf "$TARBALL" -C "$STAGE" "StashTrack-${VERSION}-linux-x86_64"
echo "built $TARBALL"
