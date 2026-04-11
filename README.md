# wurm-locator (Python)

Locate an in-game cartography map screenshot on a full server topo map.

## What it does
1. Extract the inner 500×500 map area from a Wurm client GUI screenshot.
2. Detect the black X overlay position inside that 500×500 map.
3. Remove overlays (X + compass corner) and match the cleaned patch against a full server map image.
4. Convert the best match to tile coordinates (requires server map size in tiles).
5. Undo the offset math used by treasure maps to estimate the treasure tile.

This is designed to work even when the cartography map is stylized (sepia/blur/contours),
by matching on a normalized representation (edges + blurred grayscale) and searching
multiple scales.

## Install
```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## Configure servers
Edit `configs/servers.yaml`.

Important fields per server:
- `map_image`: path to topo map image (png/jpg)
- `map_size_tiles`: tiles-per-side (e.g. Xanadu is 16384)
- `scales`: list of scale factors to try for the extracted 500×500 patch

## Run (CLI)
```bash
python -m wurm_locator locate --server xanadu --config configs/servers.yaml \
  --screenshot /path/to/Screenshot_input.jpeg --debug-dir out_debug
```

## Run (GUI)
```bash
python -m wurm_locator gui --config configs/servers.yaml
```

## Notes / limitations
- If the server's topo image is not generated from similar data/styling as the in-game map,
  matching may need tuning (scales, edge thresholds, blur, etc.).
- Terraforming or an out-of-date topo image will degrade accuracy.
