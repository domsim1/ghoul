#!/bin/bash
# setup-msys2.sh - Ghoul Language Dependencies Installer for MSYS2 UCRT64
# 
# This script checks for and installs all dependencies needed to build and test Ghoul
# on Windows using MSYS2 UCRT64 environment.
#
# Usage: ./setup-msys2.sh

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored output
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

# Check if we're running in MSYS2
check_msys2() {
    if [[ -z "$MSYSTEM" ]]; then
        print_error "This script must be run in an MSYS2 environment."
        print_error "Please open MSYS2 UCRT64 terminal and run this script."
        exit 1
    fi
    
    if [[ "$MSYSTEM" != "UCRT64" ]]; then
        print_warning "You are running in $MSYSTEM environment."
        print_warning "This script is designed for UCRT64. Continue? (y/n)"
        read -r response
        if [[ ! "$response" =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
    
    print_success "Running in MSYS2 $MSYSTEM environment"
}

# Update package database
update_packages() {
    print_status "Updating MSYS2 package database..."
    if pacman -Sy --noconfirm; then
        print_success "Package database updated"
    else
        print_error "Failed to update package database"
        exit 1
    fi
}

# Check if a package is installed
is_package_installed() {
    pacman -Qi "$1" &>/dev/null
}

# Check if a command exists
command_exists() {
    command -v "$1" &>/dev/null
}

# Check if a package exists in repositories
package_exists() {
    pacman -Si "$1" &>/dev/null
}

# Try to install one of several package alternatives
install_package_alternatives() {
    local description="$1"
    local optional="${2:-false}"
    shift 2
    local packages=("$@")
    
    # Check if any package is already installed
    for package in "${packages[@]}"; do
        if is_package_installed "$package"; then
            print_success "$description is already installed ($package)"
            return 0
        fi
    done
    
    # Try to install each package alternative
    for package in "${packages[@]}"; do
        if package_exists "$package"; then
            print_status "Installing $description ($package)..."
            if pacman -S --noconfirm "$package"; then
                print_success "$description installed successfully ($package)"
                return 0
            else
                print_warning "Failed to install $package, trying next alternative..."
            fi
        fi
    done
    
    # None of the alternatives worked
    if [[ "$optional" == "true" ]]; then
        print_warning "$description not available (tried: ${packages[*]}), skipping"
        return 0
    else
        print_error "$description not found (tried: ${packages[*]})"
        return 1
    fi
}

# Install a package if not already installed
install_package() {
    local package="$1"
    local description="$2"
    local optional="${3:-false}"
    
    if is_package_installed "$package"; then
        print_success "$description is already installed"
        return 0
    fi
    
    # Check if package exists in repository
    if ! package_exists "$package"; then
        if [[ "$optional" == "true" ]]; then
            print_warning "$description ($package) not available in repository, skipping"
            return 0
        else
            print_error "$description ($package) not found in repository"
            return 1
        fi
    fi
    
    print_status "Installing $description ($package)..."
    if pacman -S --noconfirm "$package"; then
        print_success "$description installed successfully"
    else
        if [[ "$optional" == "true" ]]; then
            print_warning "Failed to install $description (optional package installation failed)"
            return 0
        else
            print_error "Failed to install $description"
            return 1
        fi
    fi
}

# Install build dependencies
install_build_deps() {
    print_status "Installing build dependencies..."
    
    # Base development tools
    install_package "base-devel" "Base development tools"
    
    # Try different toolchain package names (varies by MSYS2 version)
    install_package_alternatives "UCRT64 toolchain" "false" \
        "mingw-w64-ucrt-x86_64-toolchain" \
        "mingw-w64-ucrt-x86_64-gcc" \
        "mingw-w64-x86_64-toolchain" \
        "mingw-w64-x86_64-gcc"
    
    # Essential build tools
    install_package_alternatives "GCC compiler" "false" \
        "mingw-w64-ucrt-x86_64-gcc" \
        "mingw-w64-x86_64-gcc"
        
    install_package_alternatives "Make build system" "false" \
        "mingw-w64-ucrt-x86_64-make" \
        "mingw-w64-x86_64-make" \
        "make"
    
    # Try to install additional toolchain components if needed
    install_package_alternatives "Windows headers" "true" \
        "mingw-w64-ucrt-x86_64-headers-git" \
        "mingw-w64-ucrt-x86_64-headers" \
        "mingw-w64-x86_64-headers-git" \
        "mingw-w64-x86_64-headers"
        
    install_package_alternatives "C runtime library" "true" \
        "mingw-w64-ucrt-x86_64-crt-git" \
        "mingw-w64-ucrt-x86_64-crt" \
        "mingw-w64-x86_64-crt-git" \
        "mingw-w64-x86_64-crt"
        
    install_package_alternatives "Windows pthread library" "true" \
        "mingw-w64-ucrt-x86_64-winpthreads-git" \
        "mingw-w64-ucrt-x86_64-winpthreads" \
        "mingw-w64-x86_64-winpthreads-git" \
        "mingw-w64-x86_64-winpthreads"
    
    # Required libraries
    install_package_alternatives "GNU Readline library" "false" \
        "mingw-w64-ucrt-x86_64-readline" \
        "mingw-w64-x86_64-readline"
        
    install_package_alternatives "libcurl library" "false" \
        "mingw-w64-ucrt-x86_64-curl" \
        "mingw-w64-x86_64-curl"
        
    install_package_alternatives "Raylib graphics library" "false" \
        "mingw-w64-ucrt-x86_64-raylib" \
        "mingw-w64-x86_64-raylib"
    
    # Optional but useful tools
    install_package_alternatives "GDB debugger" "true" \
        "mingw-w64-ucrt-x86_64-gdb" \
        "mingw-w64-x86_64-gdb"
        
    install_package_alternatives "pkg-config tool" "true" \
        "mingw-w64-ucrt-x86_64-pkg-config" \
        "mingw-w64-x86_64-pkg-config" \
        "pkg-config"
}

# Install test dependencies
install_test_deps() {
    print_status "Installing test dependencies..."
    
    install_package_alternatives "Go programming language" "false" \
        "mingw-w64-ucrt-x86_64-go" \
        "mingw-w64-x86_64-go" \
        "go"
}

# Install optional development tools  
install_optional_tools() {
    print_status "Installing optional development tools..."
    
    install_package "git" "Git version control"
    
    install_package_alternatives "CMake build system" "true" \
        "mingw-w64-ucrt-x86_64-cmake" \
        "mingw-w64-x86_64-cmake"
        
    install_package_alternatives "Ninja build system" "true" \
        "mingw-w64-ucrt-x86_64-ninja" \
        "mingw-w64-x86_64-ninja"
        
    install_package "vim" "Vim text editor"
    
    install_package_alternatives "Clang compiler (alternative to GCC)" "true" \
        "mingw-w64-ucrt-x86_64-clang" \
        "mingw-w64-x86_64-clang"
}

# Verify installations
verify_installation() {
    print_status "Verifying installations..."
    
    local errors=0
    
    # Check compilers
    if command_exists gcc; then
        print_success "GCC compiler: $(gcc --version | head -n1)"
    else
        print_error "GCC compiler not found"
        ((errors++))
    fi
    
    if command_exists clang; then
        print_success "Clang compiler: $(clang --version | head -n1)"
    else
        print_warning "Clang compiler not found (optional)"
    fi
    
    # Check build tools
    if command_exists make; then
        print_success "Make build system: $(make --version | head -n1)"
    else
        print_error "Make not found"
        ((errors++))
    fi
    
    # Check Go
    if command_exists go; then
        print_success "Go runtime: $(go version)"
    else
        print_error "Go runtime not found"
        ((errors++))
    fi
    
    # Check pkg-config for libraries
    if command_exists pkg-config; then
        # Check readline
        if pkg-config --exists readline; then
            print_success "GNU Readline library: $(pkg-config --modversion readline)"
        else
            print_error "GNU Readline library not found"
            ((errors++))
        fi
        
        # Check libcurl
        if pkg-config --exists libcurl; then
            print_success "libcurl library: $(pkg-config --modversion libcurl)"
        else
            print_error "libcurl library not found"
            ((errors++))
        fi
        
        # Check raylib
        if pkg-config --exists raylib; then
            print_success "Raylib library: $(pkg-config --modversion raylib)"
        else
            print_error "Raylib library not found"
            ((errors++))
        fi
    else
        print_warning "pkg-config not found, skipping library checks"
    fi
    
    # Check if we can find library files manually if pkg-config fails
    if ! command_exists pkg-config || ! pkg-config --exists readline; then
        if [[ -f "/ucrt64/lib/libreadline.a" ]] || [[ -f "/ucrt64/lib/libreadline.dll.a" ]]; then
            print_success "GNU Readline library files found"
        else
            print_error "GNU Readline library files not found"
            ((errors++))
        fi
    fi
    
    return $errors
}

# Test build
test_build() {
    print_status "Testing Ghoul build..."
    
    if [[ -f "Makefile" ]]; then
        if make clean && make; then
            print_success "Ghoul built successfully!"
            
            # Test the executable
            if [[ -f "ghoul.exe" ]]; then
                print_success "ghoul.exe created successfully"
                
                # Run a simple test
                if echo 'print "Hello from Ghoul!";' | ./ghoul.exe; then
                    print_success "Ghoul executable works correctly"
                else
                    print_warning "Ghoul executable may have issues"
                fi
            else
                print_error "ghoul.exe not found after build"
            fi
        else
            print_error "Failed to build Ghoul"
            print_error "Check the error messages above for missing dependencies"
            return 1
        fi
    else
        print_warning "Makefile not found. Make sure you're in the Ghoul project directory."
        return 1
    fi
}

# Main installation process
main() {
    echo "================================================="
    echo "  Ghoul Language - MSYS2 UCRT64 Setup Script"
    echo "================================================="
    echo
    
    check_msys2
    update_packages
    
    echo
    print_status "Starting dependency installation..."
    echo
    
    install_build_deps
    echo
    
    install_test_deps
    echo
    
    print_status "Would you like to install optional development tools? (y/n)"
    read -r install_optional
    if [[ "$install_optional" =~ ^[Yy]$ ]]; then
        install_optional_tools
        echo
    fi
    
    echo "================================================="
    print_status "Verifying installation..."
    echo "================================================="
    
    if verify_installation; then
        print_success "All dependencies installed successfully!"
        echo
        
        print_status "Would you like to test building Ghoul now? (y/n)"
        read -r test_build_now
        if [[ "$test_build_now" =~ ^[Yy]$ ]]; then
            echo
            test_build
        fi
        
        echo
        echo "================================================="
        print_success "Setup completed successfully!"
        echo "================================================="
        echo
        print_status "You can now build Ghoul with:"
        echo "  make"
        echo
        print_status "And run tests with:"
        echo "  make test"
        echo
        print_status "To start developing, try:"
        echo "  ./ghoul.exe"
        echo "  or"
        echo "  ./ghoul.exe your_script.ghoul"
        echo
        
    else
        echo
        print_error "Some dependencies are missing or not properly installed."
        print_error "Please check the error messages above and try running the script again."
        exit 1
    fi
}

# Handle script interruption
trap 'print_error "Setup interrupted by user"; exit 1' INT TERM

# Run main function
main "$@"