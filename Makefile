.PHONY: ghoul
ghoul:
	cc -std=c99 -Wall -Wextra -o ghoul main.c chunk.c memory.c debug.c value.c vm.c compiler.c scanner.c object.c table.c native.c -lreadline

