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
