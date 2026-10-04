#!/usr/bin/env bash
set -euo pipefail

# WurmExplorer Installer
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

INSTALL_PREFIX="${HOME}/MyApps/WurmExplorer"
BIN_DIR="${HOME}/.local/bin"
DESKTOP_DIR="${HOME}/.local/share/applications"
ICON_DIR="${HOME}/.local/share/icons/hicolor"

INSTALL_DEPS=""

# Parse optional arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix=*)
            INSTALL_PREFIX="${1#*=}"
            shift
            ;;
        --install-deps|-d)
            INSTALL_DEPS="true"
            shift
            ;;
        --no-deps)
            INSTALL_DEPS="false"
            shift
            ;;
        --help|-h)
            echo "Usage: ./install.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --prefix=PATH    Installation directory (default: $INSTALL_PREFIX)"
            echo "  --install-deps   Install system dependencies before building"
            echo "  --no-deps        Skip system dependencies prompt"
            echo "  --help, -h       Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Run ./install.sh --help for usage."
            exit 1
            ;;
    esac
done

echo "================================================="
echo "           WurmExplorer Installer"
echo "================================================="
echo "Install prefix: $INSTALL_PREFIX"
echo "Binary link:    $BIN_DIR/wurm_explorer"
echo "Desktop entry:  $DESKTOP_DIR/wurmexplorer.desktop"
echo "================================================="

# Prompt or install dependencies
if [ "$INSTALL_DEPS" = "true" ]; then
    echo ""
    echo "Installing system dependencies..."
    bash scripts/install_dependencies.sh
elif [ "$INSTALL_DEPS" = "false" ]; then
    : # Explicitly skipped by user
elif [ -t 0 ]; then
    echo ""
    read -r -p "Would you like to install system dependencies first? [y/N] " dep_choice
    if [[ "$dep_choice" =~ ^[Yy]$ ]]; then
        bash scripts/install_dependencies.sh
    fi
fi

# 1. Compile project
echo ""
echo "[1/5] Building WurmExplorer..."
if [ ! -d "builddir" ]; then
    echo "Configuring build directory..."
    meson setup builddir
fi

meson compile -C builddir

if [ ! -f "builddir/wurm_explorer" ]; then
    echo "Error: builddir/wurm_explorer binary was not found after compilation." >&2
    exit 1
fi

# 2. Prepare directories
echo "[2/5] Creating application directories..."
mkdir -p "$INSTALL_PREFIX"
mkdir -p "$BIN_DIR"
mkdir -p "$DESKTOP_DIR"

# 3. Copy application binary and resources
echo "[3/5] Installing application files..."
cp "builddir/wurm_explorer" "$INSTALL_PREFIX/wurm_explorer"
chmod +x "$INSTALL_PREFIX/wurm_explorer"

# Symlink to ~/.local/bin
ln -sf "$INSTALL_PREFIX/wurm_explorer" "$BIN_DIR/wurm_explorer"

# Copy resources
rm -rf "$INSTALL_PREFIX/resources"
cp -r "resources" "$INSTALL_PREFIX/resources"

# Copy configs template/defaults if not present in install dir
if [ -d "configs" ]; then
    mkdir -p "$INSTALL_PREFIX/configs"
    cp -n configs/*.yaml "$INSTALL_PREFIX/configs/" 2>/dev/null || true
    cp -n configs/*.json "$INSTALL_PREFIX/configs/" 2>/dev/null || true
fi

# Copy assets
if [ -d "assets" ]; then
    mkdir -p "$INSTALL_PREFIX/assets/icons"
    cp -r "assets/icons/"* "$INSTALL_PREFIX/assets/icons/" 2>/dev/null || true
fi

# 4. Install Icons to hicolor theme
echo "[4/5] Installing application icons..."
mkdir -p "$ICON_DIR/scalable/apps"
cp "assets/icons/wurm_explorer.svg" "$ICON_DIR/scalable/apps/wurm_explorer.svg"
cp "assets/icons/wurm_explorer.svg" "$ICON_DIR/scalable/apps/wurmexplorer.svg"

# Render standard raster sizes if rsvg-convert or magick is available
ICON_SIZES=(16 24 32 48 64 128 256 512)
for size in "${ICON_SIZES[@]}"; do
    TARGET_DIR="$ICON_DIR/${size}x${size}/apps"
    mkdir -p "$TARGET_DIR"
    if command -v rsvg-convert >/dev/null 2>&1; then
        rsvg-convert -w "$size" -h "$size" "assets/icons/wurm_explorer.svg" -o "$TARGET_DIR/wurm_explorer.png" 2>/dev/null || true
        rsvg-convert -w "$size" -h "$size" "assets/icons/wurm_explorer.svg" -o "$TARGET_DIR/wurmexplorer.png" 2>/dev/null || true
    elif command -v magick >/dev/null 2>&1; then
        magick -background none -resize "${size}x${size}" "assets/icons/wurm_explorer.svg" "$TARGET_DIR/wurm_explorer.png" 2>/dev/null || true
        magick -background none -resize "${size}x${size}" "assets/icons/wurm_explorer.svg" "$TARGET_DIR/wurmexplorer.png" 2>/dev/null || true
    elif [ -f "resources/icon.png" ] && [ "$size" -eq 256 ]; then
        cp "resources/icon.png" "$TARGET_DIR/wurm_explorer.png"
        cp "resources/icon.png" "$TARGET_DIR/wurmexplorer.png"
    fi
done

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t "${HOME}/.local/share/icons/hicolor" 2>/dev/null || true
fi

# 5. Install Desktop Entry
echo "[5/5] Installing desktop entry..."
cat << EOF > "$DESKTOP_DIR/wurmexplorer.desktop"
[Desktop Entry]
Version=1.0
Type=Application
Name=WurmExplorer
GenericName=Wurm Online Companion & Map
Comment=Map, navigation, breeding, and skill tracking companion for Wurm Online
Exec=$INSTALL_PREFIX/wurm_explorer %F
Path=$INSTALL_PREFIX
Icon=wurm_explorer
Terminal=false
Categories=Utility;Game;
Keywords=wurm;wurm online;map;companion;breeding;skills;cartography;explorer;
StartupWMClass=WurmExplorer
EOF

chmod +x "$DESKTOP_DIR/wurmexplorer.desktop"

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
fi

echo ""
echo "================================================="
echo " WurmExplorer successfully installed!"
echo "================================================="
echo " Executable: $INSTALL_PREFIX/wurm_explorer"
echo " Symlink:    $BIN_DIR/wurm_explorer"
echo " Desktop:    $DESKTOP_DIR/wurmexplorer.desktop"
echo ""
if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
    echo " Note: $BIN_DIR is not in your current PATH."
    echo " You can add it by appending this to your ~/.bashrc or ~/.zshrc:"
    echo "   export PATH=\"\$HOME/.local/bin:\$PATH\""
    echo ""
fi
echo " Launch from desktop app menu or run: wurm_explorer"
echo "================================================="
