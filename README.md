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

- **Linux**: GCC, Make, GNU Readline, libcurl, raylib
- **macOS**: Clang, Make, GNU Readline, raylib
- **Windows**: MSYS2 UCRT64 environment (see detailed setup below)
- **Testing**: Go runtime (for test suite)

### Quick Build

```bash
# Build the interpreter
make

# Run the REPL
./ghoul

# Run a script
./ghoul your_script.ghoul
```

### Platform-Specific Setup

#### Windows (MSYS2)
For a complete Windows development environment, see the detailed [Windows Setup Guide](SETUP_WINDOWS.md).

Quick setup:
1. Install [MSYS2](https://www.msys2.org/)
2. Run `setup-msys2.bat` or `./setup-msys2.sh` in UCRT64 terminal
3. Build with `make`

## 🧪 Testing

Ghoul includes a comprehensive test suite written in Go:

```bash
# Run all tests
make test

```

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

Ghoul is implemented as a bytecode interpreter with the following components:

### Core Components
- **Scanner** (`scanner.c`) - Lexical analysis and tokenization
- **Compiler** (`compiler.c`) - Single-pass bytecode compilation
- **Virtual Machine** (`vm.c`) - Stack-based bytecode execution
- **Memory Manager** (`memory.c`) - Garbage collection and allocation
- **Object System** (`object.c`) - Dynamic typing and object model

### Native Libraries
- **Standard Library** (`src/native/std.c`) - Core built-in functions
- **Math Library** (`src/native/math.c`) - Mathematical operations
- **JSON Library** (`src/native/json.c`) - JSON parsing via cJSON
- **HTTP Library** (`src/native/request.c`) - Network requests via libcurl
- **Graphics Library** (`src/native/raylib.c`) - Graphics and audio via raylib

## 🎯 Development Workflow

1. **Edit Code**: Use any text editor or IDE
2. **Build**: `make` for debug build, `make release` for optimized
3. **Test**: `make test` runs the complete test suite
4. **Debug**: Use `gdb ./ghoul` for debugging
5. **Clean**: `make clean` removes build artifacts

## 🔧 Configuration

### Environment Variables
- `CC` - C compiler (default: gcc)
- `CFLAGS` - Additional compiler flags
- `LDFLAGS` - Additional linker flags

### Build Options
- `make` - Debug build with symbols
- `make release` - Optimized release build
- `make clean` - Remove build artifacts
- `make test` - Run test suite

## 🤝 Contributing

While Ghoul is primarily a personal project, contributions are welcome:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## 📄 License

Ghoul is open source software. See the LICENSE file for details.

## 🙏 Acknowledgments

- **[Crafting Interpreters](https://craftinginterpreters.com/)** by Robert Nystrom - The foundation for Ghoul's implementation (CLox)
- **[raylib](https://www.raylib.com/)** - Simple and powerful graphics library
- **[cJSON](https://github.com/DaveGamble/cJSON)** - Lightweight JSON parser
- **[libcurl](https://curl.se/libcurl/)** - Versatile HTTP client library

---

