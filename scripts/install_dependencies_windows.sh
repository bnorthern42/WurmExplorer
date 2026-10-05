#!/usr/bin/env bash
# scripts/install_dependencies_windows.sh
# Dependency installer for WurmExplorer on Windows using MSYS2 (UCRT64 or MINGW64).

set -euo pipefail

AUTO_YES=""
for arg in "$@"; do
    case "$arg" in
        -y|--yes)
            AUTO_YES="--noconfirm"
            ;;
        --help|-h)
            echo "Usage: bash scripts/install_dependencies_windows.sh [OPTIONS]"
            echo ""
            echo "Must be run inside an MSYS2 UCRT64 or MINGW64 terminal on Windows."
            echo ""
            echo "Options:"
            echo "  -y, --yes    Assume yes to pacman prompts (--noconfirm)"
            echo "  -h, --help   Show this help message"
            exit 0
            ;;
    esac
done

echo "================================================="
echo " WurmExplorer Windows (MSYS2) Dependency Installer"
echo "================================================="

if ! command -v pacman >/dev/null 2>&1; then
    echo "Error: 'pacman' package manager not found." >&2
    echo "This script must be executed within an MSYS2 environment (https://www.msys2.org/)." >&2
    exit 1
fi

CURRENT_MSYSTEM="${MSYSTEM:-}"
echo "Detected MSYSTEM: ${CURRENT_MSYSTEM:-unknown}"

PREFIX=""
if [ "$CURRENT_MSYSTEM" = "UCRT64" ]; then
    PREFIX="mingw-w64-ucrt-x86_64"
elif [ "$CURRENT_MSYSTEM" = "MINGW64" ]; then
    PREFIX="mingw-w64-x86_64"
else
    echo ""
    echo "Error: Unsupported MSYS2 environment: '${CURRENT_MSYSTEM:-unknown}'" >&2
    echo "Please launch either the 'MSYS2 UCRT64' (Recommended) or 'MSYS2 MINGW64' terminal." >&2
    exit 1
fi

echo "Environment:    $CURRENT_MSYSTEM"
echo "Package Prefix: ${PREFIX}-*"
echo "================================================="
echo "Installing dependencies via pacman..."

pacman -S --needed $AUTO_YES \
    "${PREFIX}-gcc" \
    "${PREFIX}-meson" \
    "${PREFIX}-ninja" \
    "${PREFIX}-qt6-base" \
    "${PREFIX}-qt6-svg" \
    "${PREFIX}-opencv" \
    "${PREFIX}-libvips" \
    "${PREFIX}-tesseract" \
    "${PREFIX}-nlohmann-json" \
    "${PREFIX}-yaml-cpp"

echo ""
echo "================================================="
echo "Dependencies installation completed successfully!"
echo "================================================="
echo "To build WurmExplorer in MSYS2 $CURRENT_MSYSTEM:"
echo "  meson setup builddir"
echo "  ninja -C builddir"
echo "  ./builddir/wurm_explorer.exe"
echo "================================================="
