#!/bin/bash
# Repository cleanup script for Gaggimate
# Removes build artifacts, cache, and temporary files

set -e

echo "🧹 Gaggimate Repository Cleanup"
echo "================================"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counter
CLEANED=0

# Function to safely remove directories
cleanup_dir() {
    if [ -d "$1" ]; then
        echo -n "Removing $1... "
        rm -rf "$1"
        echo -e "${GREEN}✓${NC}"
        ((CLEANED++))
    fi
}

# Function to safely remove files by pattern
cleanup_pattern() {
    local pattern=$1
    local description=$2
    echo -n "Cleaning $description... "
    
    if find . -name "$pattern" -type f -delete 2>/dev/null | grep -q .; then
        echo -e "${GREEN}✓${NC}"
        ((CLEANED++))
    else
        # Still count as cleaned even if nothing found
        echo -e "${GREEN}✓${NC}"
    fi
}

# Main cleanup operations
echo ""
echo "Cleaning build directories..."
cleanup_dir "sim/build"
cleanup_dir ".pio/build"
cleanup_dir "build"

echo ""
echo "Cleaning cache directories..."
cleanup_pattern "__pycache__" "Python cache"
cleanup_pattern ".pytest_cache" "pytest cache"
cleanup_pattern ".coverage" "coverage data"

echo ""
echo "Cleaning temporary files..."
cleanup_pattern "*.pyc" "compiled Python files"
cleanup_pattern "*.pyo" "optimized Python files"
cleanup_pattern "*.swp" "Vim swap files"
cleanup_pattern "*~" "backup files"

echo ""
echo "Cleaning OS-specific files..."
find . -name ".DS_Store" -delete 2>/dev/null || true
find . -name "Thumbs.db" -delete 2>/dev/null || true

echo ""
echo -e "${GREEN}✓ Cleanup complete!${NC}"
echo ""
echo "Summary:"
echo "--------"

# Display disk usage after cleanup
echo "Repository size:"
du -sh . 2>/dev/null | awk '{print "  Total: " $1}'

if [ -d "sim" ]; then
    echo "Sim directory:"
    du -sh sim 2>/dev/null | awk '{print "  Size: " $1}'
fi

echo ""
echo "Git status:"
git status --short | wc -l | awk '{print "  Changed files: " $1}'

echo ""
echo -e "${YELLOW}Tip:${NC} Run 'git status' to verify changes before committing"
