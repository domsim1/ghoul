CC=cc
WARN=-Wall -Wextra
LIBS=-lreadline -lcurl -lm
cfiles := src/*.c
hfiles := src/*.h

vcfiles := vendor/cJSON/cJSON.c

ifeq ($(OS), Windows_NT)
    WIN_STACK=-Wl,--stack,8388608
	EXE=ghoul.exe
else
    WIN_STACK=
    EXE=ghoul
endif

ghoul: $(cfiles) $(hfiles)
	$(CC) $(WARN) -g -o $(EXE) $(cfiles) $(vcfiles) $(LIBS) $(WIN_STACK)

.PHONY: test release install uninstall clean bear
test: ghoul
	cp ./$(EXE) ./tests/$(EXE)
	cp -r ./std ./tests/
	cd ./tests; go run tests.go

release: $(cfiles) $(hfiles)
	$(CC) -DRELEASE -O2 $(WARN) -o $(EXE) $(cfiles) $(LIBS)

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
