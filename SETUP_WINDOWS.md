# Windows Development Setup for Ghoul

This guide will help you set up a complete development environment for building and testing Ghoul on Windows using MSYS2 UCRT64.

## Prerequisites

### 1. Install MSYS2

If you don't have MSYS2 installed:

1. Download MSYS2 from [https://www.msys2.org/](https://www.msys2.org/)
2. Run the installer and install to the default location (`C:\msys64\`)
3. Follow the installation instructions on the MSYS2 website
4. After installation, update the package database:
   ```bash
   pacman -Syu
   ```

## Automated Setup (Recommended)

### Option 1: Run from Windows Command Prompt
1. Double-click `setup-msys2.bat` in the Ghoul project directory
2. The script will automatically launch MSYS2 UCRT64 and install all dependencies

### Option 2: Run from MSYS2 UCRT64 Terminal
1. Open MSYS2 UCRT64 terminal (from Start Menu)
2. Navigate to the Ghoul project directory
3. Run the setup script:
   ```bash
   ./setup-msys2.sh
   ```

## What the Script Installs

### Build Dependencies
- **Base development tools**: Essential build utilities
- **UCRT64 toolchain**: GCC compiler, linker, and related tools
- **Make**: Build system
- **GNU Readline**: For interactive command line support
- **libcurl**: HTTP client library for network requests
- **Raylib**: Graphics and game development library

### Test Dependencies
- **Go**: Required for running the test suite

### Optional Development Tools
The script will ask if you want to install additional tools:
- **Git**: Version control
- **CMake & Ninja**: Alternative build systems
- **Vim**: Text editor
- **Clang**: Alternative C compiler
- **GDB**: Debugger
- **pkg-config**: Library configuration tool

## Manual Setup (Advanced Users)

If you prefer to install dependencies manually:

```bash
# Update package database
pacman -Sy

# Install build essentials
pacman -S base-devel mingw-w64-ucrt-x86_64-toolchain

# Install required libraries
pacman -S mingw-w64-ucrt-x86_64-readline
pacman -S mingw-w64-ucrt-x86_64-curl
pacman -S mingw-w64-ucrt-x86_64-raylib

# Install Go for testing
pacman -S mingw-w64-ucrt-x86_64-go

# Install make
pacman -S mingw-w64-ucrt-x86_64-make
```

## Building Ghoul

After setup is complete:

1. Open MSYS2 UCRT64 terminal
2. Navigate to the Ghoul project directory
3. Build the project:
   ```bash
   make
   ```

## Running Tests

To run the test suite:

```bash
make test
```

## Usage

After building, you can run Ghoul:

```bash
# Interactive mode (REPL)
./ghoul.exe

# Run a script file
./ghoul.exe your_script.ghoul

# Example
echo 'print "Hello, Ghoul!";' | ./ghoul.exe
```

## Troubleshooting

### Common Issues

**Error: "command not found"**
- Make sure you're using the MSYS2 UCRT64 terminal, not the regular Windows Command Prompt
- Verify that the PATH is set correctly in MSYS2

**Library not found errors**
- Re-run the setup script: `./setup-msys2.sh`
- Check that packages are installed: `pacman -Qi mingw-w64-ucrt-x86_64-raylib`

**Build failures**
- Ensure all dependencies are installed
- Try cleaning and rebuilding: `make clean && make`
- Check that you're in the correct directory with the Makefile

**Test failures**
- Make sure Go is installed: `go version`
- Verify the ghoul.exe was built successfully
- Check that test files are present in the `tests/` directory

### Getting Help

If you encounter issues:

1. Check that you're using MSYS2 UCRT64 (not MINGW64 or MSYS2)
2. Verify all dependencies are installed by running the verification section of the setup script
3. Try rebuilding from scratch: `make clean && make`
4. Check the Ghoul documentation in `docs/learn.md`

## Environment Variables

The setup script automatically configures the environment, but if you need to set things manually:

```bash
# Ensure UCRT64 tools are in PATH
export PATH="/ucrt64/bin:$PATH"

# Set compiler if needed
export CC=gcc
```

## Development Workflow

1. **Edit code**: Use your preferred editor (Vim, VS Code, etc.)
2. **Build**: `make` or `make release` for optimized build
3. **Test**: `make test` to run the full test suite
4. **Debug**: Use `gdb ./ghoul.exe` if you installed GDB
5. **Clean**: `make clean` to remove build artifacts

## IDE Integration

### Visual Studio Code
1. Install the C/C++ extension
2. Open the project folder in VS Code
3. Configure the C/C++ extension to use the UCRT64 compiler:
   - Press `Ctrl+Shift+P`
   - Type "C/C++: Edit Configurations"
   - Set compiler path to: `C:/msys64/ucrt64/bin/gcc.exe`

### CLion/Other IDEs
Configure your IDE to use the MSYS2 UCRT64 toolchain:
- Compiler: `C:\msys64\ucrt64\bin\gcc.exe`
- Make: `C:\msys64\ucrt64\bin\make.exe`
- Debugger: `C:\msys64\ucrt64\bin\gdb.exe`

This setup provides a complete, native Windows development environment for Ghoul with all necessary dependencies and tools.
