# wurm-locator (Python)

A desktop map tool for Wurm Online.

It started as a treasure-map locator and has grown into a multi-purpose map application with:

- treasure map matching
- manual annotations
- imported map data layers
- artifact search overlays
- kingdom-aware guard tower support on PvP-enabled servers
- a general-purpose drawing/planning tab for vector map markup

## Features

### Treasure locating

The locator can:

1. Extract the inner map area from a Wurm client screenshot
2. Detect the black X overlay inside that map
3. Remove overlays such as the X and compass
4. Match the cleaned patch against a full server map image
5. Convert the best match into tile coordinates
6. Undo treasure offset math to estimate the treasure tile

The matcher is designed to still work when the in-game map is stylized with blur, sepia, contours, or similar effects by using normalized matching based on edges and grayscale and by searching multiple scales.

### Desktop GUI

The GUI is built with PyQt6 and includes these tabs:

- **Annotations**
  - add and edit manual deeds, roads, bridges, and tunnels
  - on PvP-enabled servers, add manual guard towers with kingdom ownership and influence radius
  - search and filter manual annotations

- **Drawing**
  - create reusable drawing objects for planning and map markup
  - add multiple items to the same object
  - supported item types:
    - polyline
    - rectangle
    - circle
    - arrow
    - text
  - each object can contain mixed item types, colors, widths, and labels
  - text items support:
    - multi-line text
    - font family
    - font size
    - bold
    - italic
  - in pan mode, existing text items can be selected and dragged to move them
  - `Ctrl+S` saves the current item in the Drawing tab

- **Map Data**
  - browse imported read-only map layers
  - deeds
  - guard towers
  - resources
  - special points of interest
  - highways
  - bridges
  - tunnels
  - search, filter, and toggle layers

- **Artifacts**
  - track artifact locate casts
  - place a caster position
  - record clue distance bands and facing
  - intersect clue areas on the map

- **Treasure**
  - load a screenshot
  - locate the map position
  - set a manual hint
  - review top candidate matches

- **Settings**
  - adjust layer colors
  - adjust widths
  - toggle labels for imported map layers

### Map variants

The GUI supports switching between multiple map variants such as:

- classic
- topo
- terrain

If your server maps follow a naming format like:

```text
<server>-<type>-<creationDate>.png
````

the app can discover and switch between available variants automatically.

Current behavior:

* the **Treasure** tab always prefers `topo` when a topo map exists
* the other tabs prefer `terrain`
* if a preferred variant does not exist, the app falls back cleanly

### Importing external map data

The project can import external map data from a `window.sheetData` / `valueRanges` style dump, such as map sites that expose Google Sheets-backed data.

Supported imported sheets include:

* `Deeds`
* `Highways`
* `Bridges`
* `Tunnels`
* `Resources`
* `Special`

Imported data is kept separate from manual annotations in the GUI.

## Install

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

If `PyQt6` is not already included in your requirements file, install it manually:

```bash
pip install PyQt6
```

## Project layout

Important config and data files:

* `configs/servers.yaml` - server definitions
* `configs/annotations.json` - manual annotations and imported map data
* `configs/artifacts.json` - saved artifact clues
* `configs/kingdoms.json` - saved kingdom definitions for PvP servers
* `configs/styles.json` - saved GUI style settings
* `configs/drawings.json` - saved drawing/planning objects

## Configure servers

Edit `configs/servers.yaml`.

Important fields per server:

* `map_image` - path to the default server map image
* `map_size_tiles` - tiles per side
* `scales` - scale factors to try for treasure matching
* `server_mode` - `pve` or `pvp`
* `supports_kingdoms` - enable manual kingdom-owned guard towers
* `guard_tower_influence_radius_tiles` - default manual tower influence radius
* treasure-matching parameters such as:

  * `canny1`
  * `canny2`
  * `blur_ksize`
  * `score_weights`
  * `topk`

Example:

```yaml
servers:
  Xanadu:
    map_image: "../svrMaps/Xanadu-terrain-20260224.png"
    map_size_tiles: 8192
    scales: [0.55, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85, 0.90]
    server_mode: "pve"
    supports_kingdoms: false

  Chaos:
    map_image: "../svrMaps/Chaos-terrain-20260224.png"
    map_size_tiles: 2048
    scales: [0.70, 0.75, 0.80, 0.85, 0.90, 0.95, 1.0]
    server_mode: "pvp"
    supports_kingdoms: true
    guard_tower_influence_radius_tiles: 50
```

## Run

### GUI

```bash
python run_locator.py gui --config configs/servers.yaml
```

### Treasure locate from CLI

```bash
python run_locator.py locate \
  --config configs/servers.yaml \
  --server Xanadu \
  --screenshot /path/to/Screenshot_input.jpeg \
  --debug-dir out_debug
```

Optional hint arguments:

```bash
python run_locator.py locate \
  --config configs/servers.yaml \
  --server Xanadu \
  --screenshot /path/to/Screenshot_input.jpeg \
  --hint-x 4000 \
  --hint-y 2500 \
  --hint-radius 300 \
  --debug-dir out_debug
```

### Import map data from `window.sheetData`

Dry run:

```bash
python run_locator.py import-sheetdata \
  --config configs/servers.yaml \
  --server Xanadu \
  --input /path/to/xanadu.html \
  --source-name yaga-xanadu \
  --dry-run
```

Real import:

```bash
python run_locator.py import-sheetdata \
  --config configs/servers.yaml \
  --server Xanadu \
  --input /path/to/xanadu.html \
  --source-name yaga-xanadu \
  --replace-existing
```

## Imported map data behavior

Imported data is treated as read-only map data and shown through the **Map Data** tab.

Manual annotations are edited through the **Annotations** tab.

This split keeps imported site data separate from your own hand-made edits.

## PvP and kingdoms

Only servers that enable kingdom support in `servers.yaml` expose kingdom-aware guard tower editing.

On PvP-enabled servers you can:

* place manual guard towers
* assign a kingdom from a saved list
* add a new kingdom and reuse it later
* draw tower influence circles using the kingdom color

On PvE servers, kingdom-specific tower editing is hidden.

## Artifact workflow

The Artifacts tab is intended for narrowing artifact locations over multiple casts.

Each clue stores:

* artifact
* caster tile
* facing direction
* distance band

The app converts clues into geometric search areas and overlays their intersections on the map.

## Drawing workflow

The Drawing tab is intended for planning, markup, and general map sketching.

### Object model

A drawing object can contain multiple saved items, such as:

* several circles
* several arrows
* several text labels
* mixed colors
* mixed widths

This makes it possible to keep related markup grouped together as a single object.

### Text editing

Text items support:

* multi-line content
* font family selection
* font size
* bold
* italic

In **Pan** mode, you can select an existing text item by clicking near its anchor point, drag it to move it, then press **Save Item** to commit text and style changes.

### Shortcuts

Inside the Drawing tab:

* `Ctrl+S` - save current item

## Notes and limitations

* Matching quality still depends on how similar the server map image is to the in-game cartography map.
* Out-of-date server maps, heavy terraforming, or mismatched map styles can reduce treasure matching accuracy.
* Large imported datasets can reduce pan performance.
* Highway and resource imports depend on the structure of the external `window.sheetData` source. Some sites may need importer adjustments if their sheet layout differs.
* The Drawing tab currently focuses on vector-style planning and markup, not full raster painting.

## Status

This project is actively evolving beyond treasure matching into a broader Wurm Online desktop map tool.


## Note on the codebase

I do not particularly care for Python, and this project is very much a practical tool rather than a polished Python showcase.

Most of it was vibe coded with Gemini and ChatGPT, then iterated until it became useful. The focus here is getting features working for Wurm map tooling, not writing the prettiest or most idiomatic Python on earth.

So if parts of the code feel a little stitched together, that is because they are. The goal was speed, utility, and experimentation.

## Local config

This repo does not track personal runtime data or local server config.

Create your local server config from the example:

```bash
cp configs/servers.example.yaml configs/servers.yaml
```

Local saved data such as drawings, annotations, artifacts, styles, and kingdoms is stored in configs/*.json and is ignored by Git.

