#!/usr/bin/env bash
# run-container.sh — Run WurmExplorer inside Docker or Podman with GUI forwarding

set -e

# Detect container engine (prefers podman if installed, otherwise docker)
if command -v podman >/dev/null 2>&1; then
    ENGINE="podman"
elif command -v docker >/dev/null 2>&1; then
    ENGINE="docker"
else
    echo "Error: Neither podman nor docker was found in PATH." >&2
    exit 1
fi

IMAGE_NAME="wurmexplorer:latest"

# Build image if it doesn't exist
if ! $ENGINE image inspect "$IMAGE_NAME" >/dev/null 2>&1; then
    echo "Image $IMAGE_NAME not found. Building with $ENGINE..."
    $ENGINE build -t "$IMAGE_NAME" .
fi

# Enable X11 local access
if command -v xhost >/dev/null 2>&1; then
    xhost +local:$(id -un) >/dev/null 2>&1 || xhost +local: >/dev/null 2>&1 || true
fi

# Setup display environment
DISPLAY_VAL="${DISPLAY:-:0}"
X11_SOCKET="/tmp/.X11-unix"

# Device acceleration options
DEVICE_ARGS=()
if [ -d "/dev/dri" ]; then
    DEVICE_ARGS+=(--device /dev/dri)
fi

# Volume mounts for client configs and logs
VOLUME_ARGS=()
if [ -d "$HOME/.config/wurm" ]; then
    VOLUME_ARGS+=(-v "$HOME/.config/wurm:/root/.config/wurm:rw")
fi

echo "Launching WurmExplorer using $ENGINE..."

if [ "$ENGINE" = "podman" ]; then
    podman run --rm -it \
        --net=host \
        --ipc=host \
        -e DISPLAY="$DISPLAY_VAL" \
        -v "$X11_SOCKET:$X11_SOCKET:ro" \
        "${VOLUME_ARGS[@]}" \
        "${DEVICE_ARGS[@]}" \
        "$IMAGE_NAME" "$@"
else
    docker run --rm -it \
        --net=host \
        --ipc=host \
        -e DISPLAY="$DISPLAY_VAL" \
        -v "$X11_SOCKET:$X11_SOCKET:ro" \
        -v "$HOME/.Xauthority:/root/.Xauthority:ro" \
        "${VOLUME_ARGS[@]}" \
        "${DEVICE_ARGS[@]}" \
        "$IMAGE_NAME" "$@"
fi
