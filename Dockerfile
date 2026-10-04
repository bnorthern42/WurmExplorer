# Multi-stage Dockerfile for WurmExplorer
# Compatible with both Docker and Podman

# Stage 1: Build environment
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    g++ \
    meson \
    ninja-build \
    pkg-config \
    qt6-base-dev \
    qt6-base-dev-tools \
    libvips-dev \
    libopencv-dev \
    libyaml-cpp-dev \
    nlohmann-json3-dev \
    libtesseract-dev \
    libomp-dev \
    python3 \
    python3-pip \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

ENV MAP_FOLDER_ID="0B6J_aGQ6URL8UURFN2VadWxtSWs"

RUN pip3 install --no-cache-dir --break-system-packages gdown requests \
    && python3 scripts/fetch_maps.py --folder-id "${MAP_FOLDER_ID}" --dest assets/maps/

RUN meson setup builddir --buildtype=release \
    && ninja -C builddir

# Stage 2: Runtime environment with GUI support
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
ENV QT_QPA_PLATFORM=xcb

RUN apt-get update && apt-get install -y --no-install-recommends \
    qt6-base-dev \
    libvips42t64 \
    libopencv-dev \
    libyaml-cpp-dev \
    libtesseract5 \
    libomp5 \
    libx11-xcb1 \
    libxkbcommon-x11-0 \
    libgl1 \
    libglx-mesa0 \
    libgl1-mesa-dri \
    fonts-dejavu-core \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy binary, resources, configs, and map assets
COPY --from=builder /app/builddir/wurm_explorer /app/wurm_explorer
COPY --from=builder /app/resources /app/resources
COPY --from=builder /app/configs /app/configs
COPY --from=builder /app/assets /app/assets
COPY --from=builder /app/svrMaps /app/svrMaps

ENV PATH="/app:${PATH}"

CMD ["/app/wurm_explorer"]
