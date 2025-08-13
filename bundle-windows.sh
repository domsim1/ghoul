#!/bin/bash
# bundle-windows.sh - Create a portable Windows bundle of Ghoul

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in MSYS2
if [[ -z "$MSYSTEM" ]]; then
    print_error "This script must be run in MSYS2 environment"
    exit 1
fi

# Configuration
BUNDLE_DIR="ghoul-windows-bundle"
VERSION=$(date +"%Y%m%d")

print_status "Building Ghoul Windows Bundle v$VERSION"
echo

# Clean and build release version
print_status "Building release version..."
make clean
make release

if [[ ! -f "ghoul.exe" ]]; then
    print_error "Failed to build ghoul.exe"
    exit 1
fi

print_success "Release build completed"

# Create bundle directory
print_status "Creating bundle directory..."
rm -rf "$BUNDLE_DIR"
mkdir -p "$BUNDLE_DIR"

# Copy executable
print_status "Copying executable..."
cp ghoul.exe "$BUNDLE_DIR/"

# Copy standard library
print_status "Copying standard library..."
cp -r std "$BUNDLE_DIR/"

# Copy documentation
print_status "Copying documentation..."
cp README.md "$BUNDLE_DIR/"
cp LICENSE "$BUNDLE_DIR/" 2>/dev/null || print_warning "LICENSE file not found"
mkdir -p "$BUNDLE_DIR/docs"
cp -r docs/* "$BUNDLE_DIR/docs/" 2>/dev/null || print_warning "docs directory not found"


# Find and copy DLL dependencies
print_status "Finding DLL dependencies..."

# Function to copy DLL and its dependencies recursively
copy_dll_deps() {
    local file="$1"
    local target_dir="$2"
    
    # Get list of DLLs this file depends on
    local dlls=$(ldd "$file" 2>/dev/null | grep -E "\.dll" | awk '{print $1}' | sort -u)
    
    for dll in $dlls; do
        # Skip system DLLs
        if [[ "$dll" =~ ^(kernel32|user32|advapi32|shell32|ole32|oleaut32|ws2_32|winmm|gdi32|comdlg32|comctl32|msvcrt|ucrtbase)\.dll$ ]]; then
            continue
        fi
        
        # Find the actual DLL file
        local dll_path=$(ldd "$file" 2>/dev/null | grep "$dll" | awk '{print $3}' | head -n1)
        
        if [[ -f "$dll_path" && ! -f "$target_dir/$dll" ]]; then
            print_status "  Copying $dll"
            cp "$dll_path" "$target_dir/"
            
            # Recursively copy dependencies of this DLL
            copy_dll_deps "$dll_path" "$target_dir"
        fi
    done
}

# Copy DLL dependencies
copy_dll_deps "$BUNDLE_DIR/ghoul.exe" "$BUNDLE_DIR"

# Create README for the bundle
print_status "Creating bundle README..."
cat > "$BUNDLE_DIR/README-BUNDLE.txt" << EOF
Ghoul Programming Language - Windows Bundle
==========================================

This is a portable Windows distribution of Ghoul that can run without
installing MSYS2 or any development tools.

Files included:
- ghoul.exe          - The Ghoul interpreter
- std/               - Standard library files
- *.dll              - Required runtime libraries
- docs/              - Documentation

Quick Start:
1. Open Command Prompt or PowerShell in this directory
2. Run: ghoul.exe (to start REPL) or ghoul.exe your-script.ghoul

Command Line Usage:
- ghoul.exe                    - Start REPL
- ghoul.exe script.ghoul       - Run a script
- ghoul.exe --help             - Show help (if implemented)

For more information, see README.md

Bundle created: $(date)
Version: $VERSION
EOF


# Calculate bundle size
BUNDLE_SIZE=$(du -sh "$BUNDLE_DIR" | cut -f1)

echo
print_success "Bundle created successfully!"
echo
print_status "Bundle details:"
echo "  Location: $BUNDLE_DIR"
echo "  Size: $BUNDLE_SIZE"
echo "  Files: $(find "$BUNDLE_DIR" -type f | wc -l)"
echo
print_status "To test the bundle:"
echo "  1. Copy '$BUNDLE_DIR' to any Windows machine"
echo "  2. Open Command Prompt in the bundle directory"
echo "  3. Run 'ghoul.exe' to start the REPL"
echo
print_status "To create a zip archive:"
echo "  zip -r ghoul-windows-v$VERSION.zip '$BUNDLE_DIR'"
echo

print_success "Bundle creation completed!"