# Repository Cleanup Guide

This guide helps keep the Gaggimate repository clean and maintainable.

## Automated Cleanup Script

Run the following command to clean up build artifacts and temporary files:

```bash
# Clean simulator build
rm -rf sim/build

# Clean PlatformIO build
rm -rf .pio/build

# Clean Python cache
find . -type d -name __pycache__ -exec rm -rf {} + 2>/dev/null
find . -type f -name "*.pyc" -delete

# Clean OS files
find . -name ".DS_Store" -delete
find . -name "Thumbs.db" -delete
```

Or use this one-liner:

```bash
./clean.sh
```

## What Gets Cleaned

### Build Artifacts
- `sim/build/` - Emscripten simulator build output
- `.pio/build/` - PlatformIO build output
- CMake build files
- Object files (`.o`), libraries (`.a`), etc.

### Cache & Temporary Files
- `__pycache__/` directories
- `.pyc` and `.pyo` files
- Editor temporary files (`*~`, `*.swp`)

### OS-Specific Files
- `.DS_Store` (macOS)
- `Thumbs.db` (Windows)

### IDE Files
- `.vscode/` - VS Code settings (local only, not committed)
- `.idea/` - IntelliJ settings (local only, not committed)

## Before Committing

Always run cleanup before committing:

```bash
# Clean temporary files
rm -rf sim/build .pio/build

# Check what will be committed
git status

# Add and commit
git add -A
git commit -m "your message"
git push origin master
```

## Git Ignore

The `.gitignore` file prevents most temporary files from being added. Common ignored patterns:

- `/sim/build/` - Simulator build directory
- `/.pio/build/` - PlatformIO build
- `/__pycache__/` - Python cache
- `.DS_Store` - macOS metadata
- `node_modules/` - NPM dependencies
- `.venv/` - Python virtual environment

## .gitignore Structure

- **Build artifacts** - Compiled binaries and object files
- **IDEs & Editors** - Editor configuration and cache
- **Environment** - Virtual environments and dependencies
- **Generated files** - Auto-generated source code
- **OS specific** - Platform-specific temporary files

## Large Files

If you accidentally commit large files (>50MB), use git-filter-branch or BFG to remove them:

```bash
# Using BFG (recommended)
bfg --delete-files <filename>

# Push cleaned history
git push origin master --force-with-lease
```

## Disk Space

To check disk space usage:

```bash
# Largest directories
du -sh */ | sort -hr | head -10

# Simulator build size
du -sh sim/build/

# PlatformIO cache
du -sh .pio/
```

## Rebuilding After Cleanup

After running cleanup scripts, rebuild the simulator:

```bash
cd sim
source ~/emsdk/emsdk_env.sh
EMSDK_QUIET=1 ./build.sh
cd build
python3 -m http.server 8000
```

---

**Last Updated:** 2024-12-04
