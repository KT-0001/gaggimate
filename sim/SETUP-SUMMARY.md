# GaggiMate Display Simulator - Setup Summary

## What Was Created

A complete LVGL web-based simulator environment has been set up in the `sim/` directory. This allows you to test display UI changes in a web browser before deploying to hardware.

## Files Created

### Core Files
- **`sim/main.c`** - Main simulator entry point with LVGL initialization and SDL integration
- **`sim/lv_conf_simulator.h`** - LVGL configuration adapted for web/simulator environment
- **`sim/CMakeLists.txt`** - CMake build configuration for Emscripten
- **`sim/index.html`** - HTML shell for the web-based simulator
- **`sim/mouse_cursor_icon.c`** - Mouse cursor support (minimal)

### Build & Run Scripts
- **`sim/build.sh`** - Automated build script with dependency checking
- **`sim/serve.sh`** - Development web server script
- **`sim/.gitignore`** - Git ignore rules for build artifacts

### Documentation
- **`sim/README.md`** - Complete documentation (installation, usage, troubleshooting)
- **`sim/QUICKSTART.md`** - Quick reference guide for common workflows
- **`sim/README-ADDITION.md`** - Suggested addition for main project README
- **`sim/github-actions-example.yml`** - Example CI/CD workflow

## Architecture

```
┌─────────────────────────────────────────────────┐
│           Web Browser                           │
│  ┌───────────────────────────────────────────┐ │
│  │         index.html (UI Shell)             │ │
│  └───────────────────────────────────────────┘ │
│                     ↓                           │
│  ┌───────────────────────────────────────────┐ │
│  │   lvgl_sim.js (JavaScript Wrapper)        │ │
│  └───────────────────────────────────────────┘ │
│                     ↓                           │
│  ┌───────────────────────────────────────────┐ │
│  │   lvgl_sim.wasm (WebAssembly)             │ │
│  │                                             │ │
│  │  ┌─────────────────────────────────────┐  │ │
│  │  │  LVGL 8.4 (UI Library)              │  │ │
│  │  └─────────────────────────────────────┘  │ │
│  │  ┌─────────────────────────────────────┐  │ │
│  │  │  SDL2 (Display & Input via Emscr.)  │  │ │
│  │  └─────────────────────────────────────┘  │ │
│  │  ┌─────────────────────────────────────┐  │ │
│  │  │  GaggiMate UI Code (Your UI)        │  │ │
│  │  └─────────────────────────────────────┘  │ │
│  └───────────────────────────────────────────┘ │
└─────────────────────────────────────────────────┘
```

## How It Works

1. **Build Process** (`build.sh`):
   - Checks for Emscripten installation
   - Clones LVGL v8.4 if not present
   - Compiles C code to WebAssembly using Emscripten
   - Generates: `lvgl_sim.js`, `lvgl_sim.wasm`, `lvgl_sim.data`

2. **Runtime** (in browser):
   - Loads HTML shell (`index.html`)
   - Initializes WebAssembly module
   - Creates SDL2 window (480x480 to match hardware)
   - Initializes LVGL
   - Renders UI to HTML5 Canvas
   - Captures mouse events as touch input

3. **Development Workflow**:
   ```
   Edit UI files → Run build.sh → Refresh browser → Test changes
   ```

## Display Specifications

Simulator matches the LilyGo T-RGB hardware:
- **Resolution**: 480x480 pixels
- **Color Depth**: 16-bit RGB565
- **LVGL Version**: 8.4.0
- **Refresh Rate**: 10ms (configurable)

## What's Simulated

✅ **Fully Simulated**:
- LVGL rendering pipeline
- Touch input (via mouse)
- All LVGL widgets and features
- Animations and transitions
- Screen navigation
- Visual appearance

⚠️ **Partially Simulated**:
- Timing (browser-based, not real-time)
- Memory constraints (more generous in browser)

❌ **Not Simulated**:
- Hardware peripherals (sensors, Bluetooth, WiFi)
- ESP32-specific features
- SPIFFS/SD card file system
- Real-time constraints

## Next Steps to Use

### 1. Install Emscripten (One-time)
```bash
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk
./emsdk install latest
./emsdk activate latest
source ~/emsdk/emsdk_env.sh
```

### 2. Build Simulator
```bash
cd sim
./build.sh
```

### 3. Run Simulator
```bash
./serve.sh
# Opens on http://localhost:8000
```

### 4. Integrate Real UI (Optional)

Currently the simulator shows a demo UI. To use the actual GaggiMate UI:

**Option A: Modify CMakeLists.txt**
```cmake
# Add to CMakeLists.txt
file(GLOB_RECURSE UI_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/display/ui/default/lvgl/*.c
)

# Add to executable
set(SIMULATOR_SOURCES
    main.c
    mouse_cursor_icon.c
    ${UI_SOURCES}
)
```

**Option B: Create symlinks**
```bash
cd sim
ln -s ../src/display/ui/default/lvgl ui
# Update CMakeLists.txt accordingly
```

Then uncomment `ui_init()` in `main.c` line 229.

## Integration with Development

### Testing Branches
```bash
git checkout feature/new-ui
cd sim && ./build.sh && ./serve.sh
```

### Testing PRs
```bash
git fetch origin pull/123/head:pr-123
git checkout pr-123
cd sim && ./build.sh && ./serve.sh
```

### CI/CD Integration
Copy `github-actions-example.yml` to `.github/workflows/simulator.yml` to automatically build the simulator on PRs.

## Troubleshooting

See `sim/README.md` section "Troubleshooting" for detailed solutions.

Common issues:
- **"emcc not found"**: Run `source ~/emsdk/emsdk_env.sh`
- **Blank screen**: Check browser console, rebuild with `rm -rf build && ./build.sh`
- **Changes not appearing**: Hard refresh browser (Ctrl+Shift+R)

## Resources

- **LVGL Docs**: https://docs.lvgl.io/8.4/
- **LVGL Browser Integration**: https://docs.lvgl.io/9.4/integration/pc/browser.html
- **Emscripten Docs**: https://emscripten.org/docs/
- **Project Docs**: See `sim/README.md`

## Benefits

1. **Faster Development**: Test UI changes instantly without flashing hardware
2. **Safer Testing**: No risk of bricking hardware with bad code
3. **Collaboration**: Easy to share simulator builds for review
4. **CI/CD**: Automated testing of UI changes in pull requests
5. **Cross-Platform**: Test on any OS with a browser

## License

Same as main GaggiMate project (CC BY-NC-SA 4.0)

---

**Ready to start simulating! 🚀**

For questions or issues, see the detailed README.md in the sim/ directory.
