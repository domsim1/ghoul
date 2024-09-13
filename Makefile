.PHONY: clox
clox:
	cc -std=c99 -Wall -Wextra -o clox main.c chunk.c memory.c debug.c value.c vm.c compiler.c scanner.c object.c table.c native.c

