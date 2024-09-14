CC=cc
WARN=-Wall -Wextra
EXE=ghoul
LIBS=-lreadline
cfiles := main.c chunk.c memory.c debug.c value.c vm.c compiler.c scanner.c object.c table.c native.c
hfiles := common.h chunk.h compiler.h debug.h memory.h native.h object.h scanner.h table.h value.h vm.h

ghoul: $(cfiles) $(hfiles)
	$(CC) $(WARN) -o $(EXE) $(cfiles) $(LIBS)

.PHONY: test clean
test: ghoul
	cp ./ghoul ./tests/ghoul
	cd ./tests; go run tests.go

clean:
	rm ./ghoul
	rm ./tests/ghoul
