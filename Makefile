CC=cc
WARN=-Wall -Wextra
EXE=ghoul
LIBS=-lreadline -lm
cfiles := src/*.c
hfiles := src/*.h

WIN_TARGET := $(if $(findstring Windows, $(TARGET_OS)), 1, 0)
WIN_STACK := -Wl,--stack,8388608

ghoul: $(cfiles) $(hfiles)
	$(CC) $(WARN) -g -o $(EXE) $(cfiles) $(LIBS) $(if $(WIN_TARGET), $(WIN_STACK), )

.PHONY: test release install uninstall clean bear
test: ghoul
	cp $(if $(WIN_TARGET), ./ghoul.exe ./tests/ghoul.exe, ./ghoul ./tests/ghoul)
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
	rm $(if $(WIN_TARGET), ./ghoul.exe, ./ghoul) 
	rm -rf /tests/std
	rm $(if $(WIN_TARGET), ./tests/ghoul.exe, ./tests/ghoul)

bear:
	bear -- make
