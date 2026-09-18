#!/bin/bash
set -e

echo "Building WurmExplorer..."
cd "$(dirname "$0")"
cd builddir
ninja
cd ..

INSTALL_DIR="$HOME/MyApps/WurmExplorer"
DESKTOP_FILE="$HOME/.local/share/applications/wurmexplorer.desktop"

echo "Creating installation directory at $INSTALL_DIR..."
mkdir -p "$INSTALL_DIR"

echo "Copying files..."
cp builddir/wurm_locator "$INSTALL_DIR/wurm_explorer"
cp -r resources "$INSTALL_DIR/resources"

echo "Creating desktop entry..."
cat << EOF > "$DESKTOP_FILE"
[Desktop Entry]
Version=1.0
Type=Application
Name=WurmExplorer
Comment=Map and location tool for Wurm Online
Exec=$INSTALL_DIR/wurm_explorer
Icon=$INSTALL_DIR/resources/icon.png
Terminal=false
Categories=Utility;Game;
EOF

chmod +x "$DESKTOP_FILE"

echo "Installation complete!"
echo "You can now launch WurmExplorer from your application launcher."
