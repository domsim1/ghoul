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
	cd ./tests; go run tests.go

clean:
	rm ./ghoul
	rm ./tests/ghoul
