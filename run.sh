#!/bin/bash
set -e

# Ensure we are in the script's directory
cd "$(dirname "$0")"

echo "Compiling WurmExplorer..."
meson compile -C builddir

if [ $? -eq 0 ]; then
    echo "Starting WurmExplorer..."
    ./builddir/wurm_explorer "$@"
else
    echo "Compilation failed."
    exit 1
fi
