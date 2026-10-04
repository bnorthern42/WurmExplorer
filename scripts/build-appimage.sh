#!/usr/bin/env bash
# scripts/build-appimage.sh — Package WurmExplorer as an AppImage

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "=== Building WurmExplorer ==="
if [ ! -d "builddir" ]; then
    meson setup builddir --buildtype=release
fi
ninja -C builddir

echo "=== Preparing AppDir ==="
APPDIR="$ROOT_DIR/builddir/AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/lib"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/scalable/apps"

# Copy binary and resources
cp "$ROOT_DIR/builddir/wurm_explorer" "$APPDIR/usr/bin/wurm_explorer"
cp -r "$ROOT_DIR/resources" "$APPDIR/usr/bin/"
cp -r "$ROOT_DIR/configs" "$APPDIR/usr/bin/"

# Copy desktop file and icon
cp "$ROOT_DIR/resources/wurmexplorer.desktop" "$APPDIR/wurmexplorer.desktop"
cp "$ROOT_DIR/resources/wurmexplorer.desktop" "$APPDIR/usr/share/applications/wurmexplorer.desktop"
cp "$ROOT_DIR/resources/wurmexplorer.svg" "$APPDIR/wurmexplorer.svg"
cp "$ROOT_DIR/resources/wurmexplorer.svg" "$APPDIR/usr/share/icons/hicolor/scalable/apps/wurmexplorer.svg"

# AppRun entry script
cat <<'EOF' > "$APPDIR/AppRun"
#!/bin/sh
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${HERE}/usr/lib64:${LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="${HERE}/usr/plugins:${QT_PLUGIN_PATH}"
export QML2_IMPORT_PATH="${HERE}/usr/qml:${QML2_IMPORT_PATH}"

cd "${HERE}/usr/bin"
exec "${HERE}/usr/bin/wurm_explorer" "$@"
EOF
chmod +x "$APPDIR/AppRun"

echo "=== Packaging AppImage ==="
OUTPUT_APPIMAGE="${ROOT_DIR}/builddir/WurmExplorer-x86_64.AppImage"

# Download appimagetool if not present
APPIMAGETOOL="/tmp/appimagetool"
if [ ! -f "$APPIMAGETOOL" ]; then
    echo "Downloading appimagetool..."
    curl -fsSL -o "$APPIMAGETOOL" https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x "$APPIMAGETOOL"
fi

# Package AppImage (extract and run to avoid FUSE requirement in containers/CI)
export ARCH=x86_64
export APPIMAGE_EXTRACT_AND_RUN=1

"$APPIMAGETOOL" "$APPDIR" "$OUTPUT_APPIMAGE"

echo "=== Successfully created AppImage: $OUTPUT_APPIMAGE ==="
