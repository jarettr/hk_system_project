#!/usr/bin/env bash

# Heuristic OS Build & Emulation Script
# Targets: x86_64 seL4/CAmkES environment

set -e # Exit immediately if a command exits with a non-zero status

# --- Configuration ---
APP_NAME="hk_system"
PLATFORM="x86_64"
DOCKER_IMAGE="trustworthysystems/sel4:latest"
BUILD_DIR="build"

echo "[INFO] Starting Heuristic OS build process..."

# 1. Clean up environment
if [ -d "$BUILD_DIR" ]; then
    echo "[INFO] Removing existing build directory..."
    rm -rf "$BUILD_DIR"
fi
mkdir "$BUILD_DIR"

# 2. Synchronize build environment
echo "[INFO] Pulling latest build container: $DOCKER_IMAGE"
docker pull "$DOCKER_IMAGE"

# 3. Execute build inside Docker
echo "[INFO] Initializing seL4 build system and compiling..."

docker run --rm \
    -v "$(pwd)":/host \
    -w /host/"$BUILD_DIR" \
    "$DOCKER_IMAGE" \
    bash -c "
        ../init-build.sh \
            -DPLATFORM=$PLATFORM \
            -DCAMKES_APP=$APP_NAME \
            -DSIMULATION=TRUE && \
        ninja
    "

if [ $? -eq 0 ]; then
    echo "---------------------------------------------------------------"
    echo "[SUCCESS] Build complete."
    echo "[INFO] To start the simulation, run: ./$BUILD_DIR/simulate"
    echo "---------------------------------------------------------------"
else
    echo "[ERROR] Build failed. Check compiler output above."
    exit 1
fi
