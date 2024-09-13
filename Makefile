CC=cc
STD=-std=c99
WARN=-Wall -Wextra
EXE=ghoul
LIBS=-lreadline
files := main.c chunk.c memory.c debug.c value.c vm.c compiler.c scanner.c object.c table.c native.c

ghoul: $(files)
	$(CC) $(STD) $(WARN) -o $(EXE) $(files) $(LIBS)

.PHONY: test clean
test: ghoul
	cp ./ghoul ./tests/ghoul
	cd ./tests; go run tests.go

clean:
	rm ./ghoul
	rm ./tests/ghoul
