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
	WIN_LIBS=-lgdi32 -lwinmm
else
    WIN_STACK=
    DEFINES=
    EXE=ghoul
    WIN_LIBS=
endif

ghoul: $(cfiles) $(hfiles)
	$(CC) $(WARN) -g $(DEFINES) -o $(EXE) $(cfiles) $(vcfiles) $(LIBS) $(WIN_LIBS) $(WIN_STACK)

.PHONY: test release install uninstall clean bear
test: ghoul
	cp ./$(EXE) ./tests/$(EXE)
	cp -r ./std ./tests/
	cd ./tests; go run tests.go

release: $(cfiles) $(hfiles)
	$(CC) -DRELEASE $(DEFINES) -O2 $(WARN) -o $(EXE) $(cfiles) $(vcfiles) $(LIBS) $(WIN_LIBS) $(WIN_STACK)

install: release
	install -d /usr/bin
	install -m 755 ./ghoul /usr/bin
	install -d /usr/share/ghoul
	install -m 644 ./std/std.ghoul /usr/share/ghoul/

uninstall:
	rm -f /usr/bin/ghoul
	rm -rf /usr/share/ghoul

clean:
	rm ./$(EXE)
	rm -rf /tests/std
	rm ./tests/$(EXE)

bear:
	bear -- make
