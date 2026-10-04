#!/bin/bash
# ==============================================================================
# scripts/capture_docs.sh
# Automated documentation screenshot utility for Wayland / Niri compositors
# Requires: grim (Wayland image grabber)
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="$REPO_ROOT/assets/docs"
DEFAULT_DELAY=5

mkdir -p "$OUTPUT_DIR"

if ! command -v grim >/dev/null 2>&1; then
    echo "ERROR: 'grim' is not installed or not in PATH."
    echo "Please install grim (e.g., 'sudo dnf install grim' or 'sudo pacman -S grim')."
    exit 1
fi

echo "================================================="
echo " WurmExplorer Documentation Screenshot Capture"
echo " Compositor: Wayland (Niri) | Tool: grim"
echo " Destination: assets/docs/"
echo "================================================="
echo ""

capture_target() {
    local filename="$1"
    local description="$2"
    local delay="${3:-$DEFAULT_DELAY}"
    local filepath="$OUTPUT_DIR/$filename"

    echo "-------------------------------------------------"
    echo ">> Target: $description"
    echo ">> File:   assets/docs/$filename"
    echo ">> Switch to the desired tab in WurmExplorer!"
    echo ">> Capturing in $delay seconds..."

    for ((i = delay; i > 0; i--)); do
        printf "   [%d...] " "$i"
        sleep 1
    done
    printf "\n"

    grim "$filepath"
    echo "✓ Captured: $filepath ($(stat -c%s "$filepath" 2>/dev/null || stat -f%z "$filepath") bytes)"
    echo ""
}

# If arguments are passed, capture only specified targets
if [ "$#" -gt 0 ]; then
    case "$1" in
        main|main_ui)
            capture_target "main_ui.png" "Main Cartography & Navigation UI" "${2:-$DEFAULT_DELAY}"
            ;;
        sailing|sailing_cluster)
            capture_target "sailing_cluster.png" "Sailing Routes & Cluster Navigator" "${2:-$DEFAULT_DELAY}"
            ;;
        grinder)
            capture_target "grinder.png" "Mechanics Grinder Simulator" "${2:-$DEFAULT_DELAY}"
            ;;
        bridge|bridge_pillar)
            capture_target "bridge_pillar.png" "Bridge Dirt Pillar Calculator" "${2:-$DEFAULT_DELAY}"
            ;;
        imp|imp_calc)
            capture_target "imp_calc.png" "Imping Calculator & Skill Matrix" "${2:-$DEFAULT_DELAY}"
            ;;
        livestock|granger)
            capture_target "livestock.png" "Granger Livestock & Traits Evaluator" "${2:-$DEFAULT_DELAY}"
            ;;
        skills|skills_tracker)
            capture_target "skills_tracker.png" "Skills Tracker & Real-Time Log Sync" "${2:-$DEFAULT_DELAY}"
            ;;
        help|--help|-h)
            echo "Usage: $0 [target] [delay_in_seconds]"
            echo "Available targets: main, sailing, grinder, bridge, imp, livestock, skills"
            echo "Run without arguments to sequentially capture all document screenshots."
            exit 0
            ;;
        *)
            capture_target "$1.png" "$1" "${2:-$DEFAULT_DELAY}"
            ;;
    esac
    exit 0
fi

# Full sequential capture sequence
echo "Beginning sequential capture of documentation screenshots."
echo "You will have $DEFAULT_DELAY seconds between each capture to focus the appropriate tab."
read -rp "Press [Enter] when WurmExplorer is running and ready to begin..."

capture_target "main_ui.png" "1. Main Interface / Cartography & Treasure Locator" "$DEFAULT_DELAY"
capture_target "sailing_cluster.png" "2. Sailing Routes & Cluster Navigator" "$DEFAULT_DELAY"
capture_target "grinder.png" "3. Mechanics Grinder Simulation Engine" "$DEFAULT_DELAY"
capture_target "bridge_pillar.png" "4. Bridge Dirt Pillar Calculator" "$DEFAULT_DELAY"
capture_target "imp_calc.png" "5. Imping Calculator & Skill Reference Matrix" "$DEFAULT_DELAY"
capture_target "livestock.png" "6. Granger Livestock & Husbandry Evaluator" "$DEFAULT_DELAY"
capture_target "skills_tracker.png" "7. Skills Monitor & Real-Time Log Syncer" "$DEFAULT_DELAY"

echo "================================================="
echo " All documentation screenshots successfully captured!"
echo " Location: $OUTPUT_DIR"
echo "================================================="
