#!/bin/bash

# Quick Test Runner
# Runs the simulator and provides instructions for testing

echo "================================================"
echo "  GaggiMate Simulator - Quick Test"
echo "================================================"
echo ""

# Check if build exists
if [ ! -f "build/index.html" ]; then
    echo "⚠️  Build not found. Building simulator..."
    ./build.sh || exit 1
fi

# Start server in background
echo "🚀 Starting test server on http://localhost:8000"
echo ""
cd build
python3 -m http.server 8000 > /dev/null 2>&1 &
SERVER_PID=$!
cd ..

# Wait a moment for server to start
sleep 2

# Open browser (macOS)
if command -v open &> /dev/null; then
    echo "🌐 Opening browser..."
    open http://localhost:8000
else
    echo "📱 Please open: http://localhost:8000"
fi

echo ""
echo "================================================"
echo "  TESTING INSTRUCTIONS"
echo "================================================"
echo ""
echo "1. Wait for the simulator to load"
echo "2. Click the green '▶ Run Tests' button"
echo "3. Watch tests execute in real-time"
echo "4. Check results in the Test Suite panel"
echo ""
echo "✅ Green = Test passed"
echo "❌ Red = Test failed (shows expected vs actual)"
echo ""
echo "Press Ctrl+C to stop the server"
echo "================================================"
echo ""

# Wait for user to stop
trap "echo ''; echo 'Stopping server...'; kill $SERVER_PID 2>/dev/null; exit 0" INT

# Keep script running
wait $SERVER_PID
