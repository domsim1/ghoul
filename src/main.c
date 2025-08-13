#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/history.h>
#include <readline/readline.h>
#include <unistd.h>

#include "vm.h"

static char actualpath[PATH_MAX + 1];

static void repl() {
  for (;;) {
    char *input = readline("ghoul> ");

    if (input == NULL) {
      printf("\nBye!\n");
      break;
    }

    if (strlen(input) > 0) {
      add_history(input);
    }

    interpret(input, "REPL");

    free(input);
  }
}

static char *readFile(const char *path) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(fileSize + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    exit(74);
  }
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

static void runFile(const char *path) {
  char *res = realpath(path, actualpath);
  if (res == NULL) {
    fprintf(stderr, "Failed to resolve file path.\n");
    exit(74);
  }
  char *source = readFile(actualpath);
  InterpretResult result = interpret(source, actualpath);
  free(source);

  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(70);
}

static void loadStd() {
#ifdef RELEASE
  runFile("/usr/share/ghoul/std.ghoul");
#else
  runFile("std/std.ghoul");
#endif
}

int main(int argc, const char *argv[]) {
  initVM();
  loadStd();

  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    fprintf(stderr, "Usage: ghoul [path]\n");
    exit(64);
  }

  freeVM();
  return 0;
}
