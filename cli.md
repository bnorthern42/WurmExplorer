# WurmExplorer CLI Command Reference

`wurm_explorer` provides a robust command-line interface (CLI) to control window dimensions, display modes, direct tab navigation, and automated headless documentation capture.

---

## Synopsis

```bash
wurm_explorer [OPTIONS]
```

---

## Command-Line Options

### 1. Window Sizing & Display Modes

| Option | Flag | Arguments | Description |
| :--- | :--- | :--- | :--- |
| `--size` | `-s` | `<WIDTHxHEIGHT>` or `half` or `full` | Set explicit window dimensions (e.g. `1920x1080`, `2560x1440`, `1280x720`) or presets. |
| `--width` | `-w` | `<pixels>` | Set initial window width in pixels (e.g. `-w 1280`). |
| `--height` | `-H` | `<pixels>` | Set initial window height in pixels (e.g. `-H 720`). |
| `--fullscreen` | `-f` | *(none)* | Launch the application in borderless fullscreen mode. |
| `--maximized` | `-m` | *(none)* | Launch the application maximized within the desktop workspace. |
| `--half-screen` | | *(none)* | Shortcut for launching at half-screen width (`1280x1440`), ideal for side-by-side tiling with Wurm Online. |

### 2. Navigation & Direct Tab Launch

Launch directly into a specific module without navigating the sidebar manually:

| Option | Flag | Arguments | Target Workspace |
| :--- | :--- | :--- | :--- |
| `--tab` | `-t` | `locator` or `0` | **Treasure Locator** (CV Template Matcher) |
| `--tab` | `-t` | `drawing` or `1` | **Map Drawing & Vector Canvas** |
| `--tab` | `-t` | `annotations` or `2` | **Annotations & POI Manager** |
| `--tab` | `-t` | `artifacts` or `3` | **Artifact Clue Solver** |
| `--tab` | `-t` | `data` or `4` | **Imported Map Data Overlays** |
| `--tab` | `-t` | `sailing` or `5` | **Sailing Routes & Cluster Navigator** |
| `--tab` | `-t` | `livestock` / `granger` or `6` | **Granger Livestock & Breeding Evaluator** |
| `--tab` | `-t` | `skills` or `7` | **Skills Tracker & Live Log Syncer** |
| `--tab` | `-t` | `tools` / `imp` / `bridge` / `grinder` or `8` | **Workbench: Simulators & Calculators** |

### 3. Screenshot & Documentation Automation

| Option | Arguments | Description |
| :--- | :--- | :--- |
| `--capture-docs` | `[output_directory]` | Automatically captures high-resolution screenshots across all primary panels and exits. Defaults to `assets/docs`. |

### 4. General & Help

| Option | Flag | Description |
| :--- | :--- | :--- |
| `--help` | `-h`, `-?` | Displays CLI syntax, flags, and option descriptions. |
| `--version` | `-v` | Outputs the current application version. |

---

## Practical Examples

### Launch Side-by-Side (Half Screen)
Tile alongside the Wurm Online client on a 1440p monitor:
```bash
wurm_explorer --half-screen -t skills
```
Or with custom half-screen dimensions on 1080p:
```bash
wurm_explorer -s 960x1080 -t livestock
```

### Launch Fullscreen on 1440p / 4K
```bash
wurm_explorer --fullscreen
```

### Launch Standard 1080p Windowed
```bash
wurm_explorer --size 1920x1080 -t sailing
```

### Open Directly to Mechanics Grinder or Imp Calculator
```bash
wurm_explorer -t tools
```

### Automated Documentation Screenshot Capture
Generate fresh screenshots for docs and README into `assets/docs`:
```bash
wurm_explorer --capture-docs assets/docs
```

---

## Compositor & Display Server Tips

- **Wayland (Niri / Sway / Hyprland):**
  WurmExplorer runs natively on Wayland via Qt6. Window resizing through `--size` and `--fullscreen` respect Wayland window management protocols.
  ```bash
  QT_QPA_PLATFORM=wayland wurm_explorer --size 1280x1440
  ```

- **X11 / XWayland Fallback:**
  If needed, specify the X11 platform plugin:
  ```bash
  QT_QPA_PLATFORM=xcb wurm_explorer --maximized
  ```
