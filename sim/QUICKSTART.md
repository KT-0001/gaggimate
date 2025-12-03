# Quick Start Guide - GaggiMate Display Simulator

## For Testing Branches/Forks Before Deployment

### First Time Setup (5 minutes)

1. **Install Emscripten** (one-time setup):
```bash
# Clone Emscripten SDK
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk

# Install and activate
./emsdk install latest
./emsdk activate latest

# Add to your shell profile (~/.zshrc or ~/.bashrc)
echo 'source "$HOME/emsdk/emsdk_env.sh"' >> ~/.zshrc
source ~/.zshrc
```

2. **Verify installation**:
```bash
emcc --version  # Should show Emscripten version
```

### Testing Your Changes

#### For Your Branch:
```bash
# Make UI changes in src/display/ui/
git checkout -b feature/my-ui-improvement

# Edit files in src/display/ui/default/lvgl/
# ... make your changes ...

# Build and test simulator
cd sim
./build.sh
./serve.sh

# Open http://localhost:8000 in browser
# Test your changes interactively
```

#### For Testing a Fork:
```bash
# Clone the fork
git clone https://github.com/username/gaggimate-fork.git
cd gaggimate-fork

# Build and run simulator
cd sim
./build.sh
./serve.sh

# Open http://localhost:8000
```

#### For Testing Pull Requests:
```bash
# Fetch the PR
git fetch origin pull/123/head:pr-123
git checkout pr-123

# Test in simulator
cd sim
./build.sh
./serve.sh
```

### Typical Workflow

```bash
# 1. Make changes to UI
vim src/display/ui/default/lvgl/screens/ui_BrewScreen.c

# 2. Rebuild simulator (from sim/ directory)
./build.sh

# 3. If server is running, just refresh browser
# If not running, start server:
./serve.sh

# 4. Test in browser at http://localhost:8000
```

### What Gets Simulated

✅ **Works in Simulator:**
- UI layout and styling
- Touch interactions (use mouse)
- Animations and transitions
- Screen navigation
- LVGL widgets and components
- Visual appearance

❌ **Not Simulated:**
- Temperature/pressure sensors
- Bluetooth connectivity
- WiFi connectivity  
- Real hardware timings
- SPIFFS/SD card filesystem
- ESP32-specific features

### Troubleshooting

**"emcc not found"**
```bash
source ~/emsdk/emsdk_env.sh
```

**Blank screen in browser**
- Check browser console (F12) for errors
- Rebuild: `rm -rf build && ./build.sh`
- Try different browser (Chrome recommended)

**Changes not showing**
- Hard refresh: Ctrl+Shift+R (or Cmd+Shift+R on Mac)
- Rebuild: `./build.sh`

### Tips

- Keep the server running and just rebuild when making changes
- Use browser DevTools (F12) to debug
- Check the main README.md in sim/ for advanced options
- Test on actual hardware before final deployment

### Next Steps

Once simulator testing passes:
1. Build for real hardware: `pio run -e display`
2. Upload to device: `pio run -e display -t upload`
3. Test on actual hardware
4. Create PR if working correctly

---

**Questions?** See `sim/README.md` for detailed documentation.
