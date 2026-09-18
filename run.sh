#!/bin/bash

# Ensure we are in the script's directory
cd "$(dirname "$0")"

echo "Compiling WurmLocator..."
meson compile -C builddir

if [ $? -eq 0 ]; then
    echo "Starting WurmLocator..."
    ./builddir/wurm_locator "$@"
else
    echo "Compilation failed."
    exit 1
fi
