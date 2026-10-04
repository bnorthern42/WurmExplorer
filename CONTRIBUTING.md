# Contributing to WurmExplorer

Thank you for your interest in contributing to WurmExplorer! We welcome bug reports, feature requests, documentation improvements, and code contributions.

## Code of Conduct

All contributors are expected to uphold our [Code of Conduct](CODE_OF_CONDUCT.md). Please read it before participating.

---

## Development Setup

### 1. Prerequisites

WurmExplorer is built with modern C++23 and Qt6. You will need:
- **Compiler:** GCC 13+ or Clang 17+ with C++23 standard support (`-std=c++23`)
- **Build System:** `meson` (>= 1.2.0) and `ninja`
- **Libraries:**
  - Qt6 (`qt6-base-dev`, `qt6-base-dev-tools`)
  - OpenCV 4 (`libopencv-dev`)
  - libvips (`libvips-dev`)
  - yaml-cpp (`libyaml-cpp-dev`)
  - nlohmann-json3 (`nlohmann-json3-dev`)
  - Tesseract OCR (`libtesseract-dev`)
  - OpenMP (`libomp-dev`)
- **Tools:** `clang-format` (for code formatting)

On Debian/Ubuntu:
```bash
sudo apt update && sudo apt install -y \
  build-essential g++ meson ninja-build pkg-config \
  qt6-base-dev qt6-base-dev-tools libvips-dev libopencv-dev \
  libyaml-cpp-dev nlohmann-json3-dev libtesseract-dev libomp-dev \
  clang-format
```

### 2. Clone the Repository

```bash
git clone git@github.com:bnorthern42/WurmExplorer.git
cd WurmExplorer
```

### 3. Build with Meson

Configure and build using `meson` and `ninja`:

```bash
# Configure the build directory
meson setup builddir --buildtype=debug

# Compile the project
ninja -C builddir
```

### 4. Running the Test Suite

```bash
meson test -C builddir --verbose
```

---

## Coding Standards & Guidelines

1. **C++23 Standard**: Write modern, clean C++23. Prefer standard library algorithms, ranges, and smart pointers.
2. **Code Formatting**: Format all C++ code using `clang-format` before submitting:
   ```bash
   clang-format -i src/**/*.cpp src/**/*.hpp tests/**/*.cpp
   ```
3. **Module Length**: Keep individual source files under 500 lines of code. Refactor large files into cohesive sub-modules when needed.
4. **Design System & Theme Tokens**: All UI widgets must strictly adhere to the Dark Slate & Emerald design system defined in `src/ui/ThemeTokens.hpp` (`treasure::ui::theme`). Do not introduce ad-hoc colors or legacy palettes.
5. **Test-Driven Development (TDD)**:
   - For bug fixes or new features, write a test reproducing the problem or verifying the feature in `tests/`.
   - Ensure the test fails, then implement the fix until the test turns green.

---

## Submitting a Pull Request

1. Create a feature branch from `main`:
   ```bash
   git checkout -b feature/your-feature-name
   ```
2. Commit your changes following [Conventional Commits](https://www.conventionalcommits.org/):
   - `feat:` for new capabilities
   - `fix:` for bug fixes
   - `docs:` for documentation
   - `refactor:` for code restructuring without behavior change
   - `test:` for test additions/updates
3. Ensure the test suite passes locally (`meson test -C builddir --verbose`).
4. Push your branch and open a Pull Request against `main`. Describe your changes and link any related issues.
