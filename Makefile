CC=cc
WARN=-Wall -Wextra
EXE=ghoul
LIBS=-lreadline -lm
cfiles := src/*.c
hfiles := src/*.h

ghoul: $(cfiles) $(hfiles)
	$(CC) $(WARN) -o $(EXE) $(cfiles) $(LIBS)

.PHONY: test clean
test: ghoul
	cp ./ghoul ./tests/ghoul
	cp -r ./std ./tests/
	cd ./tests; go run tests.go

clean:
	rm ./ghoul
	rm -rf /tests/std
	rm ./tests/ghoul
