#!/usr/bin/env zsh
set -euo pipefail

# Simple helper to build and serve the LVGL simulator, then open it.

SCRIPT_DIR=${0:A:h}
BUILD_DIR="$SCRIPT_DIR/build"
PORT=${PORT:-8000}

echo "[serve] Sourcing emsdk environment (if available)"
if [[ -f "$HOME/emsdk/emsdk_env.sh" ]]; then
  source "$HOME/emsdk/emsdk_env.sh"
fi

echo "[serve] Building simulator"
cd "$SCRIPT_DIR"
./build.sh

echo "[serve] Preparing index.html"
cd "$BUILD_DIR"
cp -f lvgl_sim.html index.html

echo "[serve] Checking port $PORT"
if lsof -i ":$PORT" | grep -q LISTEN; then
  pid=$(lsof -ti ":$PORT")
  echo "[serve] Killing existing server on port $PORT (pid $pid)"
  kill "$pid" || true
  sleep 1
fi

echo "[serve] Starting server on port $PORT"
python3 -m http.server "$PORT" &
srv_pid=$!
sleep 1

echo "[serve] Opening http://localhost:$PORT"
open "http://localhost:$PORT"

echo "[serve] Server pid: $srv_pid (press Ctrl+C to stop if foreground)"
wait $srv_pid || true
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
