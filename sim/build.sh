#!/bin/bash

# Build script for GaggiMate Display Simulator
# This script builds the LVGL simulator for web browsers using Emscripten

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "================================================"
echo "  GaggiMate Display Simulator Build Script"
echo "================================================"
echo ""

# Check if Emscripten is installed
if ! command -v emcc &> /dev/null; then
    echo -e "${RED}Error: Emscripten (emcc) is not found in PATH${NC}"
    echo ""
    echo "Please install Emscripten SDK:"
    echo "  1. git clone https://github.com/emscripten-core/emsdk.git"
    echo "  2. cd emsdk"
    echo "  3. ./emsdk install latest"
    echo "  4. ./emsdk activate latest"
    echo "  5. source ./emsdk_env.sh"
    echo ""
    exit 1
fi

echo -e "${GREEN}✓${NC} Emscripten found: $(emcc --version | head -n1)"

# Check if LVGL exists
LVGL_DIR="$SCRIPT_DIR/../.pio/libdeps/display/lvgl"
if [ ! -d "$LVGL_DIR" ]; then
    LVGL_DIR="$SCRIPT_DIR/lvgl"
    if [ ! -d "$LVGL_DIR" ]; then
        echo -e "${YELLOW}LVGL not found. Cloning LVGL v8.4...${NC}"
        git clone --depth 1 --branch release/v8.4 https://github.com/lvgl/lvgl.git "$LVGL_DIR"
        echo -e "${GREEN}✓${NC} LVGL cloned successfully"
    fi
fi

echo -e "${GREEN}✓${NC} LVGL found at: $LVGL_DIR"

# Create build directory
if [ -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Cleaning existing build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "Configuring CMake with Emscripten..."
emcmake cmake ..

echo ""
echo "Building simulator..."
emmake make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo -e "${GREEN}================================================${NC}"
echo -e "${GREEN}  Build completed successfully!${NC}"
echo -e "${GREEN}================================================${NC}"
echo ""
echo "To run the simulator:"
echo "  1. cd $BUILD_DIR"
echo "  2. python3 -m http.server 8000"
echo "  3. Open http://localhost:8000 in your browser"
echo ""
echo "Or run: ./serve.sh"
echo ""
