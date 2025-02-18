#!/bin/bash

# This script builds Jolt Physics from source using GCC for Release.

# Set the script's current directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set build and install directories
BUILD_DIR="$SCRIPT_DIR/build-release"
INSTALL_DIR="$SCRIPT_DIR/install-release"

# Set Jolt's actual source location (CMakeLists.txt is inside the Build folder)
JOLT_SOURCE="$SCRIPT_DIR/Build"

# Record start time
TIME_START=$(date +%T)

# Create the build directory if it doesn't exist
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || {
    echo "[ERROR] Failed to access build directory: $BUILD_DIR"
    exit 1
}

# Configure Jolt with CMake (Release)
echo "[INFO] Configuring Jolt (Release) with CMake..."
cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=g++ \
  -DCMAKE_CXX_FLAGS="-O2 -flto -DNDEBUG -Wall -Wextra -Wno-error=maybe-uninitialized" \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
  -DBUILD_SHARED_LIBS=ON \
  "$JOLT_SOURCE" || {
    echo "[ERROR] CMake configuration failed (Release)."
    exit 1
}

# Build Jolt with Make (Release)
echo "[INFO] Building Jolt (Release)..."
make -j"$(nproc)" || {
    echo "[ERROR] Build process failed (Release)."
    exit 1
}

# Install Jolt (Release)
echo "[INFO] Installing Jolt (Release)..."
make install || {
    echo "[ERROR] Install process failed (Release)."
    exit 1
}

# Record end time
TIME_END=$(date +%T)

# Success message
echo "[SUCCESS] Jolt (Release) built and installed successfully."
echo "---------------------------------------------------"
echo "Static library: $INSTALL_DIR/lib/jolt.so"
echo "Include headers: $INSTALL_DIR/include/jolt"
echo "Build duration: $TIME_START - $TIME_END"
echo "---------------------------------------------------"

exit 0
