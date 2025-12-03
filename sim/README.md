# GaggiMate Display Simulator

This directory contains a web-based simulator for the GaggiMate display UI, allowing you to test UI changes in a browser before deploying to hardware.

## Overview

The simulator uses [LVGL](https://lvgl.io/) compiled to WebAssembly with [Emscripten](https://emscripten.org/), providing an accurate representation of how the UI will look and behave on the actual LilyGo T-RGB hardware.

### Features

- **Browser-based**: No hardware required for testing
- **Interactive**: Click and drag to simulate touch input
- **Accurate rendering**: Same LVGL version (8.4) as hardware
- **Fast iteration**: Test UI changes instantly
- **480x480 display**: Matches the LilyGo T-RGB resolution

## Prerequisites

### Required Software

1. **Emscripten SDK** - For compiling to WebAssembly
2. **CMake** (3.13 or later)
3. **Python 3** - For running the local web server
4. **Git** - For cloning dependencies

### Installing Emscripten

```bash
# Clone the Emscripten SDK
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Install and activate the latest version
./emsdk install latest
./emsdk activate latest

# Set up environment variables (add to your .bashrc or .zshrc)
source ./emsdk_env.sh
```

Verify installation:
```bash
emcc --version
```

## Quick Start

### 1. Build the Simulator

```bash
cd sim
./build.sh
```

The build script will:
- Check for Emscripten installation
- Clone LVGL if needed (v8.4)
- Configure CMake with Emscripten
- Compile the simulator to WebAssembly

### 2. Run the Simulator

```bash
./serve.sh
```

Or manually:
```bash
cd build
python3 -m http.server 8000
```

### 3. Open in Browser

Navigate to: **http://localhost:8000**

The simulator should load and display the GaggiMate UI.

## Development Workflow

### Testing UI Changes

1. **Make changes** to UI files in `src/display/ui/`
2. **Rebuild** the simulator: `./build.sh`
3. **Refresh** your browser (Ctrl+R or Cmd+R)
4. **Test** the changes interactively

### Testing on a Branch or Fork

```bash
# Switch to your feature branch
git checkout feature/my-ui-changes

# Make your changes to src/display/ui/

# Build and test the simulator
cd sim
./build.sh
./serve.sh

# Open http://localhost:8000 and verify changes
```

### Integration with CI/CD

The simulator can be built in CI pipelines to automatically verify UI changes:

```yaml
# Example GitHub Actions workflow
- name: Build Display Simulator
  run: |
    cd sim
    source /path/to/emsdk/emsdk_env.sh
    ./build.sh

- name: Deploy Simulator Preview
  run: |
    # Deploy to GitHub Pages, Netlify, or other hosting
    cp -r sim/build/* public/
```

## Project Structure

```
sim/
├── README.md                   # This file
├── build.sh                    # Build script
├── serve.sh                    # Development server script
├── CMakeLists.txt              # CMake configuration
├── lv_conf_simulator.h         # LVGL configuration for simulator
├── index.html                  # HTML shell for the simulator
├── main.c                      # Simulator entry point
├── mouse_cursor_icon.c         # Mouse cursor (optional)
├── lvgl/                       # LVGL library (auto-cloned)
└── build/                      # Build output (gitignored)
    ├── lvgl_sim.js             # Compiled JavaScript
    ├── lvgl_sim.wasm           # WebAssembly binary
    └── lvgl_sim.data           # Preloaded assets
```

## Configuration

### Display Settings

The simulator is configured to match the LilyGo T-RGB hardware:
- **Resolution**: 480x480 pixels
- **Color depth**: 16-bit (RGB565)
- **LVGL version**: 8.4

To modify these settings, edit:
- `main.c` - Change `DISP_HOR_RES` and `DISP_VER_RES`
- `lv_conf_simulator.h` - Adjust LVGL features

### LVGL Configuration

The simulator uses `lv_conf_simulator.h` which mirrors the hardware configuration in `src/display/lv_conf.h` with simulator-specific adjustments:

- Standard `malloc/free` instead of ESP32 PSRAM
- Emscripten tick source instead of Arduino `millis()`
- Logging enabled for debugging
- Larger memory allocation (128KB vs 48KB)

## Linking the Actual UI

The current simulator includes a demo UI. To use the actual GaggiMate UI:

### Option 1: Copy UI Files (Simple)

```bash
cd sim
mkdir -p ui
cp -r ../src/display/ui/default/lvgl/* ui/
```

Then modify `CMakeLists.txt` to include the UI sources.

### Option 2: Symbolic Links (Advanced)

```bash
cd sim
ln -s ../src/display/ui/default/lvgl ui
```

### Option 3: Modify CMakeLists.txt

Edit `CMakeLists.txt` to include UI sources:

```cmake
# Add UI sources
file(GLOB_RECURSE UI_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/display/ui/default/lvgl/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/display/ui/default/lvgl/*.cpp
)

set(SIMULATOR_SOURCES
    main.c
    mouse_cursor_icon.c
    ${UI_SOURCES}
)
```

Uncomment `ui_init()` in `main.c`.

## Troubleshooting

### Build Fails: "emcc not found"

**Solution**: Ensure Emscripten is installed and activated:
```bash
source /path/to/emsdk/emsdk_env.sh
```

### Build Fails: "LVGL not found"

**Solution**: Run the build script which auto-clones LVGL:
```bash
./build.sh
```

Or manually clone:
```bash
git clone --depth 1 --branch release/v8.4 https://github.com/lvgl/lvgl.git sim/lvgl
```

### Simulator Shows Blank Screen

**Possible causes**:
1. JavaScript errors - Check browser console (F12)
2. WASM not loading - Verify `lvgl_sim.wasm` exists in `build/`
3. Assets missing - Check that `lvgl_sim.data` is present

**Solution**: Clear browser cache and reload, or rebuild:
```bash
cd sim
rm -rf build
./build.sh
```

### Touch/Mouse Input Not Working

**Solution**: Make sure you're clicking within the canvas area. Check browser console for errors.

### Simulator Runs Slowly

**Possible causes**:
1. Debug build - Release builds are faster
2. Browser limitations - Try Chrome/Edge for better WebAssembly performance

**Solution**: Add optimization flags in `CMakeLists.txt`:
```cmake
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
```

## Advanced Usage

### Custom Build Flags

Edit `CMakeLists.txt` to add custom Emscripten flags:

```cmake
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
    -s ASSERTIONS=1 \
    -s SAFE_HEAP=1 \
    ")
```

### Adding Assets

To include images or fonts:

1. Place assets in a `data/` directory
2. Modify `CMakeLists.txt` to preload them:
```cmake
--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/data@/data
```

### Debugging

Enable verbose logging in `lv_conf_simulator.h`:
```c
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_TRACE
```

Check browser console (F12) for LVGL log output.

### Performance Profiling

Use browser DevTools Performance tab to profile:
1. Open DevTools (F12)
2. Go to Performance tab
3. Click Record
4. Interact with simulator
5. Stop recording and analyze

## Limitations

- **No real hardware peripherals**: Temperature sensors, pressure sensors, etc. are not simulated
- **No Bluetooth/WiFi**: Network features won't work
- **Limited file system**: SPIFFS/SD card access is simulated
- **Performance differences**: Browser performance may differ from ESP32

## Contributing

When contributing UI changes:

1. Test in the simulator first
2. Verify on actual hardware before merging
3. Update simulator if you add new LVGL features
4. Keep `lv_conf_simulator.h` in sync with `src/display/lv_conf.h`

## Resources

- [LVGL Documentation](https://docs.lvgl.io/)
- [LVGL Web Simulator Guide](https://docs.lvgl.io/9.4/integration/pc/browser.html)
- [Emscripten Documentation](https://emscripten.org/docs/getting_started/index.html)
- [GaggiMate Main Repo](https://github.com/gaggimate/gaggimate)

## License

Same as the main GaggiMate project.

## Support

For issues with the simulator:
1. Check this README
2. Search existing GitHub issues
3. Open a new issue with:
   - Browser and version
   - Build output/errors
   - Steps to reproduce

---

**Happy simulating! ☕**
