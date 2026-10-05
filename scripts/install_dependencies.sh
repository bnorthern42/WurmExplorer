#!/usr/bin/env bash
# scripts/install_dependencies.sh
# Universal dependency installer for WurmExplorer across major Linux distributions.

set -euo pipefail

# Parse optional arguments
AUTO_YES=""
for arg in "$@"; do
    case "$arg" in
        -y|--yes)
            AUTO_YES="-y"
            ;;
        --help|-h)
            echo "Usage: ./scripts/install_dependencies.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -y, --yes    Assume yes to package manager installation prompts"
            echo "  -h, --help   Show this help message"
            exit 0
            ;;
    esac
done

# macOS Detection (Homebrew)
if [ "$(uname -s)" = "Darwin" ]; then
    echo "================================================="
    echo " WurmExplorer Universal Dependency Installer"
    echo "================================================="
    echo "Detected OS: macOS (Darwin)"
    echo "================================================="

    if ! command -v brew >/dev/null 2>&1; then
        echo "Warning: Homebrew ('brew') is not installed or not in your PATH." >&2
        echo "Please install Homebrew from https://brew.sh/ and re-run this script." >&2
        exit 1
    fi

    echo "Installing dependencies for macOS using Homebrew..."
    brew install gcc meson ninja qt6 opencv vips tesseract nlohmann-json yaml-cpp

    echo ""
    echo "Dependencies installation completed successfully."
    exit 0
fi

if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS="${ID:-}"
    OS_LIKE="${ID_LIKE:-}"
else
    echo "Cannot detect OS (/etc/os-release not found). Please install dependencies manually." >&2
    exit 1
fi

echo "================================================="
echo " WurmExplorer Universal Dependency Installer"
echo "================================================="
echo "Detected OS: $OS"
if [ -n "$OS_LIKE" ]; then
    echo "OS Family:   $OS_LIKE"
fi
echo "================================================="

# Privilege elevation helper
SUDO="sudo"
if [ "${EUID:-$(id -u)}" -eq 0 ]; then
    SUDO=""
fi

if [[ "$OS" == "arch" || "$OS" == "endeavouros" || "$OS" == "manjaro" || "$OS" == "garuda" || "$OS_LIKE" == *"arch"* ]]; then
    echo "Installing dependencies for Arch Linux / derivative using pacman..."
    PACMAN_CONFIRM=""
    if [ -n "$AUTO_YES" ]; then
        PACMAN_CONFIRM="--noconfirm"
    fi
    $SUDO pacman -S --needed $PACMAN_CONFIRM \
        gcc meson ninja pkgconf \
        qt6-base qt6-svg \
        opencv libvips \
        tesseract tesseract-data-eng \
        nlohmann-json yaml-cpp

elif [[ "$OS" == "fedora" || "$OS" == "ultramarine" || "$OS_LIKE" == *"fedora"* || "$OS_LIKE" == *"rhel"* ]]; then
    echo "Installing dependencies for Fedora / Ultramarine / RHEL family using dnf..."
    $SUDO dnf install $AUTO_YES \
        gcc-c++ meson ninja-build pkgconf-pkg-config \
        qt6-qtbase-devel qt6-qtsvg-devel \
        opencv-devel vips-devel \
        tesseract-devel \
        nlohmann-json-devel yaml-cpp-devel

elif [[ "$OS" == "ubuntu" || "$OS" == "debian" || "$OS" == "linuxmint" || "$OS" == "pop" || "$OS_LIKE" == *"debian"* || "$OS_LIKE" == *"ubuntu"* ]]; then
    echo "Installing dependencies for Debian / Ubuntu family using apt-get..."
    $SUDO apt-get update
    $SUDO apt-get install $AUTO_YES \
        build-essential g++ meson ninja-build pkg-config \
        qt6-base-dev qt6-base-dev-tools libqt6svg6-dev \
        libopencv-dev libvips-dev \
        libtesseract-dev tesseract-ocr-eng \
        nlohmann-json3-dev libyaml-cpp-dev libomp-dev

elif [[ "$OS" == "opensuse"* || "$OS" == "suse" || "$OS_LIKE" == *"suse"* || "$OS_LIKE" == *"opensuse"* ]]; then
    echo "Installing dependencies for openSUSE / SUSE family using zypper..."
    $SUDO zypper install $AUTO_YES \
        gcc-c++ meson ninja pkg-config \
        qt6-base-devel qt6-svg-devel \
        opencv-devel libvips-devel \
        tesseract-ocr-devel \
        nlohmann_json-devel yaml-cpp-devel

elif [[ "$OS" == "void" || "$OS_LIKE" == *"void"* ]]; then
    echo "Installing dependencies for Void Linux using xbps-install..."
    XBPS_YES=""
    if [ -n "$AUTO_YES" ]; then
        XBPS_YES="-y"
    fi
    $SUDO xbps-install -S $XBPS_YES \
        base-devel gcc meson ninja pkg-config \
        qt6-base-devel qt6-svg-devel \
        opencv-devel libvips-devel \
        tesseract-ocr-devel \
        nlohmann-json yaml-cpp-devel

elif [[ "$OS" == "gentoo" || "$OS_LIKE" == *"gentoo"* ]]; then
    echo "Installing dependencies for Gentoo using emerge..."
    GENTOO_ASK="--ask"
    if [ -n "$AUTO_YES" ]; then
        GENTOO_ASK=""
    fi
    $SUDO emerge $GENTOO_ASK --verbose \
        dev-build/meson dev-build/ninja dev-util/pkgconf \
        dev-qt/qtbase:6 dev-qt/qtsvg:6 \
        media-libs/opencv media-libs/vips \
        app-text/tesseract \
        dev-cpp/nlohmann_json dev-cpp/yaml-cpp

elif [[ "$OS" == "nixos" || "$OS_LIKE" == *"nix"* || -d "/nix" ]]; then
    echo "Nix / NixOS detected."
    echo ""
    echo "To enter a shell with all required WurmExplorer dependencies, run:"
    echo "  nix-shell -p gcc meson ninja pkg-config qt6.qtbase qt6.qtsvg opencv vips tesseract nlohmann_json yaml-cpp"
    echo ""
    echo "Or if you have a local shell.nix:"
    echo "  nix-shell"
    echo ""
    if [ -t 0 ] && [ -z "$AUTO_YES" ]; then
        read -r -p "Launch nix-shell now? [y/N] " launch_nix
        if [[ "$launch_nix" =~ ^[Yy]$ ]]; then
            exec nix-shell -p gcc meson ninja pkg-config qt6.qtbase qt6.qtsvg opencv vips tesseract nlohmann_json yaml-cpp
        fi
    fi

else
    echo "================================================="
    echo "Unsupported or unrecognized distribution: $OS"
    echo "================================================="
    echo "Please install the following dependencies manually for your distribution:"
    echo "  - C++23 compiler (GCC >= 13 or Clang >= 17)"
    echo "  - Build system: Meson, Ninja, pkg-config"
    echo "  - Qt6: Base & SVG development headers (qt6-base, qt6-svg)"
    echo "  - Core Libraries: OpenCV 4, libvips, Tesseract OCR, nlohmann_json, yaml-cpp"
    echo "================================================="
    exit 1
fi

echo ""
echo "Dependencies installation completed successfully."
