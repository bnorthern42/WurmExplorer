#!/bin/bash

# Navigate to the script's directory so it can be run from anywhere
cd "$(dirname "$0")" || exit 1

# Activate the virtual environment
if [ -f ".venv/bin/activate" ]; then
    source .venv/bin/activate
else
    echo "Error: Virtual environment not found. Please ensure dependencies are installed."
    exit 1
fi

# Run the GUI application
python3 -m wurm_locator.cli gui --config configs/servers.yaml
