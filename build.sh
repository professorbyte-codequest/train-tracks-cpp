#!/usr/bin/env bash

# build.sh: configure, build with CMake, and run all unit tests

set -euo pipefail

# Determine project root (directory containing this script)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
INSTALL_DIR="$BUILD_DIR"

# Create (or reuse) build directory
echo "==> Creating build directory: ${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# Configure the project with CMake
echo "==> Configuring project with CMake"
cmake -S . -B "${BUILD_DIR}"

cd "${BUILD_DIR}"

# Build all targets (using all available cores)
echo "==> Building project"
cmake --build . -- -j"$(nproc)"

# Run all unit tests via CTest
echo "==> Running unit tests"
ctest --output-on-failure

# Install targets into install directory
echo "==> Installing project into: $INSTALL_DIR"
cmake --install . --prefix "$INSTALL_DIR"

# Done
echo "==> Build, tests, and install completed successfully!"
