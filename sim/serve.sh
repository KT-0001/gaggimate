#!/bin/bash

# Serve the simulator on a local web server
# This script starts a simple HTTP server to run the simulator

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}Error: Build directory not found${NC}"
    echo "Please run ./build.sh first"
    exit 1
fi

if [ ! -f "$BUILD_DIR/lvgl_sim.js" ]; then
    echo -e "${RED}Error: Simulator not built${NC}"
    echo "Please run ./build.sh first"
    exit 1
fi

echo "================================================"
echo "  Starting GaggiMate Display Simulator Server"
echo "================================================"
echo ""
echo -e "${GREEN}Server starting on http://localhost:8000${NC}"
echo ""
echo "Open your browser to: http://localhost:8000"
echo ""
echo "Press Ctrl+C to stop the server"
echo ""

cd "$BUILD_DIR"

# Try to use Python 3, fall back to Python 2 if not available
if command -v python3 &> /dev/null; then
    python3 -m http.server 8000
elif command -v python &> /dev/null; then
    python -m SimpleHTTPServer 8000
else
    echo -e "${RED}Error: Python not found${NC}"
    echo "Please install Python to run the web server"
    exit 1
fi
