# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.5] - 2026-10-04

### Fixed
- Updated Meson build to gracefully fallback to `opencv` when `opencv4` is unavailable, fixing Windows MSYS2 compilation against OpenCV 5.0.

## [0.2.4] - 2026-10-04

### Fixed
- Added `cmake` to dependency installers to allow Meson to correctly resolve OpenCV on Windows and macOS.

## [0.2.3] - 2026-10-04

### Fixed
- Corrected Tesseract package name (`tesseract-ocr`) in the MSYS2 Windows dependency installer to fix CI build failures.

## [0.2.2] - 2026-10-04

### Added
- Cross-platform multi-OS build matrix in GitHub Actions (Windows, macOS, Ubuntu).
- Universal `scripts/install_dependencies.sh` script for macOS (Homebrew) and major Linux distributions.
- Windows-specific `scripts/install_dependencies_windows.sh` with smart MSYS2 environment detection (UCRT64 and MINGW64).
- Platform support disclaimer in the README calling for community testing on Windows and macOS.

## [0.2.1] - 2026-10-04

### Fixed
- Removed obsolete `svrMaps` COPY instruction from Dockerfile to fix CI build failures.

## [0.2.0] - 2026-10-04

### Added
- Live character skill syncing and fuzzy search in the Imp Calculator (`ImpCalculatorWidget`).
- PvP Server Rules toggle in Bridge Pillar calculator to dynamically halve slope limits.
- Epic and Northern cluster topologies for the sailing routing engine (`ClusterLayout`).
- Custom Wurm-themed SVG application icon featuring an isometric terrain tile with tooling accents.
- Automated `grim` Wayland screenshot script (`scripts/capture_docs.sh`) and headless capture CLI (`--capture-docs`).
- Comprehensive README overhaul with feature-driven copy and high-resolution screenshot showcases.
- CLI options for window geometry sizing (`--size`, `--half-screen`, `--fullscreen`, `--maximized`), direct tab launch (`--tab`), and command reference guide (`cli.md`).
- Universal Linux dependency installation script (`scripts/install_dependencies.sh`) supporting Arch, Fedora/Ultramarine, Debian/Ubuntu, openSUSE, Void, Gentoo, and Nix, hooked into `install.sh`.
- Full suite of open-source community health files (`LICENSE`, `CODE_OF_CONDUCT.md`, `CONTRIBUTING.md`, `SECURITY.md`, issue templates).
- Attributed `libvips` (`<vips/vips8>`) in the Architecture & Tech Stack documentation.

### Changed
- Google Drive map fetcher (`scripts/fetch_maps.py`) optimized to filter historical archives with `--target-year`, cutting download time drastically.
- GitHub Actions CI pipeline restricted to trigger only on tagged releases and manual dispatches.
- Redesigned Imp Calculator and Granger Livestock panels into responsive two-column master-detail views optimized for both half-screen tiling and 1440p fullscreen displays.
- Sailing cluster plans interface restructured with side-by-side button grid eliminating control clipping.

### Fixed
- Bridge Pillar math refactored to use Chebyshev distance for accurate Wurm Online square pyramid footprints and correct tile-to-corner matrix sizing.
- Sailing Map layout engine mathematically centers servers of differing dimensions (e.g., Melody vs Harmony).
- Escaped keyboard mnemonic ampersands (`&`) across navigation tabs and group boxes to eliminate unwanted accelerator underlines.

### Removed
- Redundant quick-settings buttons from the Skills Monitor header.

## [0.1.0] - 2026-10-04

### Added
- Initial project release with Computer Vision Treasure Locator, Map Drawing & Canvas, and Annotations Manager.
- Mechanics Grinder Simulator with Monte Carlo probability simulation for 14 Wurm Online action modes.
- Bridge Dirt Pillar Calculator and basic terraforming slope models.
- Granger Livestock breeding evaluator and trait compatibility scoring.
- Skills Tracker with real-time Wurm Online client log tailing and session gain statistics.
- Dark Slate & Emerald design system and ThemeTokens.
- AppImage packaging and Docker container build workflows.

[Unreleased]: https://github.com/bnorthern42/WurmExplorer/compare/v0.2.5...HEAD
[0.2.5]: https://github.com/bnorthern42/WurmExplorer/compare/v0.2.4...v0.2.5
[0.2.4]: https://github.com/bnorthern42/WurmExplorer/compare/v0.2.3...v0.2.4
[0.2.3]: https://github.com/bnorthern42/WurmExplorer/compare/v0.2.2...v0.2.3
[0.2.2]: https://github.com/bnorthern42/WurmExplorer/compare/v0.2.1...v0.2.2
[0.2.1]: https://github.com/bnorthern42/WurmExplorer/compare/v0.2.0...v0.2.1
[0.2.0]: https://github.com/bnorthern42/WurmExplorer/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/bnorthern42/WurmExplorer/releases/tag/v0.1.0
