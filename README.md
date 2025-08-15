# Ghoul Programming Language

Ghoul is a modern scripting language designed to be **terse**, **simple**, **expressive**, and **fast**.

> **Note**: Ghoul is crafted for personal use and experimentation. While functional and feature-complete, it's not intended for production environments.

## 🚀 Quick Example

```ghoul
# Function definition with closures
:make_counter(start) {
  :count = start;
  -> :(){
    count += 1;
    -> count;
  }
}

# Object-oriented programming
:Greeter {
  init(name) {
    this.name = name;
  }

  greet(to) {
    print "Hello " ++ to ++ ", I'm " ++ this.name ++ "!";
  }
}

# Usage
:counter = make_counter(10);
print counter(); # Output: 11
print counter(); # Output: 12

:bot = Greeter("Ghoul");
bot.greet("World"); # Output: Hello World, I'm Ghoul!
```

## 🛠️ Getting Started

### Prerequisites

Ghoul builds on **Linux**, **macOS**, and **Windows** with automatic dependency detection:

- **Linux**: GCC/Clang, Make, GNU Readline, libcurl, raylib, OpenGL libraries
- **macOS**: Clang, Make, Homebrew (for dependencies), raylib, readline
- **Windows**: MSYS2 UCRT64 environment with MinGW-w64 toolchain
- **Testing**: Go runtime (for test suite)

### Quick Build

```bash
# Build the interpreter
make

# Run the REPL
./ghoul

# Run a script
./ghoul your_script.ghoul

# View build configuration
make config
```

### Platform-Specific Setup

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential libreadline-dev libcurl4-openssl-dev libraylib-dev
make
```

#### Linux (Fedora/RHEL)
```bash
sudo dnf install gcc make readline-devel libcurl-devel raylib-devel
make
```

#### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install raylib readline curl

# Build (dependencies auto-detected)
make
```

#### Windows (MSYS2)
For a complete Windows development environment, see the detailed [Windows Setup Guide](SETUP_WINDOWS.md).

Quick setup:
1. Install [MSYS2](https://www.msys2.org/)
2. Run `setup-msys2.bat` or `./setup-msys2.sh` in UCRT64 terminal
3. Build with `make`

### Installation

```bash
# System-wide installation (Linux/macOS)
sudo make install

# Uninstall
sudo make uninstall

# Check installation
ghoul --version  # (if version flag is implemented)
```

## 🧪 Testing & Development

### Testing
```bash
# Run all tests
make test

# Build with debug symbols and sanitizers
make debug

# Build optimized release
make release
```

### IDE Support
```bash
# Generate compile_commands.json for LSP/IDE support
make compiledb
```

### Available Make Targets
- `make` or `make ghoul` - Build debug version (default)
- `make release` - Build optimized version
- `make debug` - Build with sanitizers and debug flags
- `make test` - Build and run test suite
- `make clean` - Remove build artifacts
- `make install` - Install system-wide (requires sudo)
- `make uninstall` - Remove system installation
- `make config` - Show current build configuration
- `make compiledb` - Generate IDE support files
- `make help` - Show all available targets

## 📚 Language Documentation

### Core Language
- **[Learn Ghoul](docs/learn.md)** - Complete language tutorial and reference

### Built-in Libraries

| Library | Description | Key Features |
|---------|-------------|--------------|
| **Math** | Mathematical functions | Trigonometry, logarithms, constants |
| **JSON** | JSON processing | Parse and generate JSON data |
| **Request** | Network requests | GET/POST requests via libcurl |
| **RL** | Graphics & Audio | 2D/3D graphics, input handling, audio playback |

### Example Usage

```ghoul
# HTTP requests
use "Request";
use "JSON";
:headers = ["User-Agent: Ghoul-Example/1.0"];
:response = Request.get("https://api.github.com/users/octocat", headers);
:user = JSON.parse(response.response);
print user["name"];

# Graphics programming
use "RL";
RL.init_window(800, 600, "My Game");
while (!RL.window_should_close()) {
    RL.begin_drawing();
    RL.clear_background(RL.BLACK);
    RL.draw_text("Hello Ghoul!", 10, 10, 20, RL.WHITE);
    RL.end_drawing();
}
RL.close_window();
```

## 🏗️ Architecture

Ghoul is implemented as a bytecode interpreter with robust cross-platform support:

### Core Components
- **Scanner** (`scanner.c`) - Lexical analysis and tokenization
- **Compiler** (`compiler.c`) - Single-pass bytecode compilation
- **Virtual Machine** (`vm.c`) - Stack-based bytecode execution
- **Memory Manager** (`memory.c`) - Garbage collection and allocation
- **Object System** (`object.c`) - Dynamic typing and object model
- **Platform Layer** (`main.c`) - Cross-platform executable path resolution

### Native Libraries
- **Standard Library** (`src/native/std.c`) - Core built-in functions
- **Math Library** (`src/native/math.c`) - Mathematical operations
- **JSON Library** (`src/native/json.c`) - JSON parsing via cJSON
- **HTTP Library** (`src/native/request.c`) - Network requests via libcurl
- **Graphics Library** (`src/native/raylib.c`) - Graphics and audio via raylib

### Cross-Platform Features
- **Automatic dependency detection** for Homebrew on macOS
- **Robust executable path resolution** on Windows, macOS, and Linux
- **Smart standard library location** with fallback search paths
- **Platform-specific optimizations** and library linking
- **Comprehensive error reporting** with helpful installation guidance

## 🎯 Development Workflow

1. **Edit Code**: Use any text editor or IDE (run `make compiledb` for LSP support)
2. **Build**: `make` for debug, `make release` for optimized
3. **Test**: `make test` runs the complete test suite
4. **Debug**: `make debug` builds with sanitizers, use `gdb ./ghoul` for debugging
5. **Install**: `sudo make install` for system-wide installation
6. **Clean**: `make clean` removes build artifacts

## 🔧 Configuration

### Environment Variables
- `CC` - C compiler (default: cc, falls back to gcc/clang)
- `CFLAGS` - Additional compiler flags
- `LDFLAGS` - Additional linker flags

### Build Customization
```bash
# Use specific compiler
CC=clang make

# Add custom flags
CFLAGS="-march=native" make release

# View current configuration
make config
```

## 🐛 Troubleshooting

### Common Issues

**"Failed to resolve file path"** after installation:
- Ensure `sudo make install` completed successfully
- Standard library should be in `/usr/local/share/ghoul/std/`

**Missing dependencies on Ubuntu/Debian:**
```bash
sudo apt install build-essential libreadline-dev libcurl4-openssl-dev libgl1-mesa-dev libx11-dev
```

**Homebrew issues on macOS:**
```bash
# Reinstall dependencies
brew uninstall raylib readline
brew install raylib readline
make clean && make
```

**Windows build issues:**
- Ensure you're using MSYS2 UCRT64 environment
- Run the setup script: `./setup-msys2.sh`

## 🤝 Contributing

While Ghoul is primarily a personal project, contributions are welcome:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass (`make test`)
6. Submit a pull request

## 📄 License

Ghoul is open source software. See the LICENSE file for details.

## 🙏 Acknowledgments

- **[Crafting Interpreters](https://craftinginterpreters.com/)** by Robert Nystrom - The foundation for Ghoul's implementation (CLox)
- **[raylib](https://www.raylib.com/)** - Simple and powerful graphics library
- **[cJSON](https://github.com/DaveGamble/cJSON)** - Lightweight JSON parser
- **[libcurl](https://curl.se/libcurl/)** - Versatile HTTP client library

---