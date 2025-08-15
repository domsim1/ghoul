CC=cc
WARN=-Wall -Wextra
BASE_LIBS=-lreadline -lm -lcurl -lraylib
cfiles := $(wildcard src/*.c src/native/*.c)
hfiles := $(wildcard src/*.h src/native/*.h)
vcfiles := src/external/cJSON/cJSON.c

# Detect OS
UNAME_S := $(shell uname -s)
ifeq ($(OS), Windows_NT)
  WIN_STACK=-Wl,--stack,8388608
  DEFINES=-DNOGDI -DNOUSER -DWIN32_LEAN_AND_MEAN
  EXE=ghoul.exe
  OS_LIBS=-lgdi32 -lwinmm
  LIBS=$(BASE_LIBS) $(OS_LIBS)
else
  WIN_STACK=
  DEFINES=
  EXE=ghoul
  ifeq ($(UNAME_S), Darwin)
    # macOS - Check if Homebrew exists and use it
    BREW_PREFIX := $(shell command -v brew >/dev/null 2>&1 && brew --prefix 2>/dev/null)
    ifneq ($(BREW_PREFIX),)
      RAYLIB_PREFIX := $(shell brew --prefix raylib 2>/dev/null)
      ifneq ($(RAYLIB_PREFIX),)
        CFLAGS += -I$(RAYLIB_PREFIX)/include
        LDFLAGS += -L$(RAYLIB_PREFIX)/lib
      endif
      # Also check for readline via Homebrew
      READLINE_PREFIX := $(shell brew --prefix readline 2>/dev/null)
      ifneq ($(READLINE_PREFIX),)
        CFLAGS += -I$(READLINE_PREFIX)/include
        LDFLAGS += -L$(READLINE_PREFIX)/lib
      endif
    endif
    OS_LIBS=-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    LIBS=$(BASE_LIBS) $(OS_LIBS)
  else
    # Linux and other Unix-like systems
    OS_LIBS=-lGL -lpthread -ldl -lrt -lX11
    LIBS=$(BASE_LIBS) $(OS_LIBS)
  endif
endif

# Default target
.DEFAULT_GOAL := ghoul

# Main build target
ghoul: $(cfiles) $(hfiles) $(vcfiles)
	$(CC) $(WARN) -g $(DEFINES) $(CFLAGS) -o $(EXE) $(cfiles) $(vcfiles) $(LIBS) $(LDFLAGS) $(WIN_STACK)

# Test target
.PHONY: test
test: ghoul
	@mkdir -p ./tests
	cp ./$(EXE) ./tests/$(EXE)
	cp -r ./std ./tests/ 2>/dev/null || true
	cd ./tests && go run tests.go

# Release build
.PHONY: release
release: $(cfiles) $(hfiles) $(vcfiles)
	$(CC) -DRELEASE $(DEFINES) $(CFLAGS) -O2 $(WARN) -o $(EXE) $(cfiles) $(vcfiles) $(LIBS) $(LDFLAGS) $(WIN_STACK)

# Clean target
.PHONY: clean
clean:
	rm -f ./$(EXE)
	rm -rf ./tests/std
	rm -f ./tests/$(EXE)

# Install target (optional)
.PHONY: install
install: ghoul
	@echo "Installing $(EXE) to /usr/local/bin (requires sudo)"
	sudo cp $(EXE) /usr/local/bin/
	@echo "Creating standard library directory"
	sudo mkdir -p /usr/local/share/ghoul/std
	sudo cp -r std/* /usr/local/share/ghoul/std/

# Uninstall target (optional)
.PHONY: uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(EXE)
	sudo rm -rf /usr/local/share/ghoul

# Development database for IDEs
.PHONY: compiledb
compiledb:
	@command -v compiledb >/dev/null 2>&1 || (echo "compiledb not found. Install via: python -m pip install compiledb" && exit 1)
	compiledb make clean
	compiledb make

# Debug build with additional flags
.PHONY: debug
debug: CFLAGS += -DDEBUG -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
debug: LDFLAGS += -fsanitize=address -fsanitize=undefined
debug: $(cfiles) $(hfiles) $(vcfiles)
	$(CC) $(WARN) -g $(DEFINES) $(CFLAGS) -o $(EXE) $(cfiles) $(vcfiles) $(LIBS) $(LDFLAGS) $(WIN_STACK)


# Show build configuration
.PHONY: config
config:
	@echo "Build configuration:"
	@echo "  OS: $(UNAME_S)"
	@echo "  CC: $(CC)"
	@echo "  CFLAGS: $(CFLAGS)"
	@echo "  LDFLAGS: $(LDFLAGS)"
	@echo "  LIBS: $(LIBS)"
	@echo "  DEFINES: $(DEFINES)"
	@echo "  EXE: $(EXE)"

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  ghoul     - Build the main executable (default)"
	@echo "  release   - Build optimized release version"
	@echo "  debug     - Build with debug flags and sanitizers"
	@echo "  test      - Build and run tests"
	@echo "  clean     - Remove build artifacts"
	@echo "  install   - Install to system (requires sudo)"
	@echo "  uninstall - Remove from system (requires sudo)"
	@echo "  compiledb - Generate compile_commands.json for IDEs"
	@echo "  config    - Show build configuration"
	@echo "  help      - Show this help message"