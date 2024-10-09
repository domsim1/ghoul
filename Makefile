CC=cc
WARN=-Wall -Wextra
EXE=ghoul
LIBS=-lreadline -lm
cfiles := src/*.c
hfiles := src/*.h

ghoul: $(cfiles) $(hfiles)
	$(CC) $(WARN) -o $(EXE) $(cfiles) $(LIBS)

.PHONY: test release install uninstall clean bear
test: ghoul
	cp ./ghoul ./tests/ghoul
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
	rm ./ghoul
	rm -rf /tests/std
	rm ./tests/ghoul

bear:
	bear -- make
