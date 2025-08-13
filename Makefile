CC=cc
WARN=-Wall -Wextra
LIBS=-lreadline -lm -lcurl -lraylib
cfiles := src/*.c src/native/*.c
hfiles := src/*.h src/native/*.c

vcfiles := src/external/cJSON/cJSON.c

ifeq ($(OS), Windows_NT)
    WIN_STACK=-Wl,--stack,8388608
    DEFINES=-DNOGDI -DNOUSER -DWIN32_LEAN_AND_MEAN
	EXE=ghoul.exe
	OS_LIBS=-lgdi32 -lwinmm
else
    WIN_STACK=
    DEFINES=
    EXE=ghoul
    OS_LIBS=-lGL -lm -lpthread -ldl -lrt -lX11
endif

ghoul: $(cfiles) $(hfiles)
	$(CC) $(WARN) -g $(DEFINES) -o $(EXE) $(cfiles) $(vcfiles) $(LIBS) $(OS_LIBS) $(WIN_STACK)

.PHONY: test release install uninstall clean compiledb
test: ghoul
	cp ./$(EXE) ./tests/$(EXE)
	cp -r ./std ./tests/
	cd ./tests; go run tests.go

release: $(cfiles) $(hfiles)
	$(CC) -DRELEASE $(DEFINES) -O2 $(WARN) -o $(EXE) $(cfiles) $(vcfiles) $(LIBS) $(OS_LIBS) $(WIN_STACK)

install: release
	install -d /usr/bin
	install -m 755 ./ghoul /usr/bin
	install -d /usr/share/ghoul
	install -m 644 ./std/std.ghoul /usr/share/ghoul/

uninstall:
	rm -f /usr/bin/ghoul
	rm -rf /usr/share/ghoul

clean:
	rm -f ./$(EXE)
	rm -rf ./tests/std
	rm -f ./tests/$(EXE)

compiledb:
	@which compiledb > /dev/null 2>&1 || (echo "compiledb not found. Install via: python -m pip install compiledb" && exit 1)
	compiledb make clean
	compiledb make
