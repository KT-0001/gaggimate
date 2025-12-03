# Display Simulator

A web-based simulator is available in the `sim/` directory for testing UI changes before deploying to hardware.

## Quick Start

```bash
# One-time setup: Install Emscripten
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest
source ~/emsdk/emsdk_env.sh

# Build and run simulator
cd sim
./build.sh
./serve.sh

# Open http://localhost:8000 in your browser
```

See [`sim/README.md`](sim/README.md) for complete documentation.

### Why Use the Simulator?

- ✅ Test UI changes without hardware
- ✅ Faster iteration cycle
- ✅ Test branches and forks before merging
- ✅ Interactive browser-based preview
- ✅ CI/CD integration ready

---

*Add this section to the main README.md under "Development" or "Contributing"*
