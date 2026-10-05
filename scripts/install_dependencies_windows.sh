#!/usr/bin/env bash
# scripts/install_dependencies_windows.sh
# Dependency installer for WurmExplorer on Windows using MSYS2 UCRT64 environment.

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
            echo "Must be run inside an MSYS2 UCRT64 terminal on Windows."
            echo ""
            echo "Options:"
            echo "  -y, --yes    Assume yes to pacman prompts (--noconfirm)"
            echo "  -h, --help   Show this help message"
            exit 0
            ;;
    esac
done

echo "================================================="
echo " WurmExplorer Windows (MSYS2 UCRT64) Installer"
echo "================================================="

if ! command -v pacman >/dev/null 2>&1; then
    echo "Error: 'pacman' package manager not found." >&2
    echo "This script must be executed within an MSYS2 environment (https://www.msys2.org/)." >&2
    exit 1
fi

CURRENT_MSYSTEM="${MSYSTEM:-unknown}"
echo "Detected MSYSTEM: $CURRENT_MSYSTEM"

if [ "$CURRENT_MSYSTEM" != "UCRT64" ]; then
    echo ""
    echo "Warning: You are currently running in '$CURRENT_MSYSTEM' instead of 'UCRT64'." >&2
    echo "For full ISO C++23, GCC 13+, and modern Qt6 compatibility, please launch" >&2
    echo "'MSYS2 UCRT64' from the Windows Start menu or run 'ucrt64.exe'." >&2
    echo ""
fi

echo "Installing UCRT64 toolchain and dependencies via pacman..."
echo "================================================="

pacman -S --needed $AUTO_YES \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-meson \
    mingw-w64-ucrt-x86_64-ninja \
    mingw-w64-ucrt-x86_64-qt6-base \
    mingw-w64-ucrt-x86_64-qt6-svg \
    mingw-w64-ucrt-x86_64-opencv \
    mingw-w64-ucrt-x86_64-libvips \
    mingw-w64-ucrt-x86_64-tesseract \
    mingw-w64-ucrt-x86_64-nlohmann-json \
    mingw-w64-ucrt-x86_64-yaml-cpp

echo ""
echo "================================================="
echo "Dependencies installation completed successfully!"
echo "================================================="
echo "To build WurmExplorer in MSYS2 UCRT64:"
echo "  meson setup builddir"
echo "  ninja -C builddir"
echo "  ./builddir/wurm_explorer.exe"
echo "================================================="
