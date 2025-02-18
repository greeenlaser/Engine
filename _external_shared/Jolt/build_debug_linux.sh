#!/bin/bash

# This script builds Jolt Physics from source using GCC for Debug.

# Set the script's current directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set build and install directories
BUILD_DIR="$SCRIPT_DIR/build-debug"
INSTALL_DIR="$SCRIPT_DIR/install-debug"

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

# Configure Jolt with CMake (Debug)
echo "[INFO] Configuring Jolt (Debug) with CMake..."
cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=g++ \
  -DCMAKE_CXX_FLAGS="-O0 -g -DDEBUG -Wall -Wextra -Wno-error=maybe-uninitialized" \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
  -DBUILD_SHARED_LIBS=ON \
  "$JOLT_SOURCE" || {
    echo "[ERROR] CMake configuration failed (Debug)."
    exit 1
}

# Build Jolt with Make (Debug)
echo "[INFO] Building Jolt (Debug)..."
make -j"$(nproc)" || {
    echo "[ERROR] Build process failed (Debug)."
    exit 1
}

# Install Jolt (Debug)
echo "[INFO] Installing Jolt (Debug)..."
make install || {
    echo "[ERROR] Install process failed (Debug)."
    exit 1
}

# Record end time
TIME_END=$(date +%T)

# Success message
echo "[SUCCESS] Jolt (Debug) built and installed successfully."
echo "---------------------------------------------------"
echo "Static library: $INSTALL_DIR/lib/jolt.so"
echo "Include headers: $INSTALL_DIR/include/jolt"
echo "Build duration: $TIME_START - $TIME_END"
echo "---------------------------------------------------"

exit 0
