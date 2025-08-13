#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/history.h>
#include <readline/readline.h>
#include <unistd.h>

#include "vm.h"

static char actualpath[PATH_MAX + 1];

static int get_nesting_level(const char *input) {
  int brace_count = 0;
  int paren_count = 0;
  int in_string = 0;
  char string_char = 0;
  
  for (const char *p = input; *p; p++) {
    if (!in_string) {
      if (*p == '"' || *p == '\'') {
        in_string = 1;
        string_char = *p;
      } else if (*p == '{') {
        brace_count++;
      } else if (*p == '}') {
        brace_count--;
      } else if (*p == '(') {
        paren_count++;
      } else if (*p == ')') {
        paren_count--;
      }
    } else {
      if (*p == string_char && (p == input || *(p-1) != '\\')) {
        in_string = 0;
      }
    }
  }
  
  return brace_count + paren_count + (in_string ? 1 : 0);
}

static char* create_prompt(int nesting_level) {
  if (nesting_level == 0) {
    char *prompt = malloc(8);
    if (prompt == NULL) {
      return NULL;
    }
    strcpy(prompt, "ghoul> ");
    return prompt;
  }
  
  int base_indent = 7;
  int additional_indent = nesting_level * 2;
  int total_spaces = base_indent + additional_indent;
  int prompt_len = total_spaces + 3;
  char *prompt = malloc(prompt_len);
  if (prompt == NULL) {
    return NULL;
  }
  
  for (int i = 0; i < total_spaces; i++) {
    prompt[i] = ' ';
  }
  prompt[total_spaces] = '.';
  prompt[total_spaces + 1] = ' ';
  prompt[total_spaces + 2] = '\0';
  
  return prompt;
}

static void repl() {
  char *full_input = NULL;
  size_t full_input_size = 0;
  
  for (;;) {
    int nesting_level = (full_input == NULL) ? 0 : get_nesting_level(full_input);
    char *prompt = create_prompt(nesting_level);
    if (prompt == NULL) {
      fprintf(stderr, "Memory allocation failed for prompt\n");
      if (full_input) {
        free(full_input);
      }
      break;
    }
    char *input = readline(prompt);
    free(prompt);

    if (input == NULL) {
      if (full_input) {
        free(full_input);
        full_input = NULL;
      }
      printf("\nBye!\n");
      break;
    }

    if (full_input == NULL) {
      size_t input_len = strlen(input);
      full_input = malloc(input_len + 1);
      if (full_input == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(input);
        break;
      }
      strcpy(full_input, input);
      full_input_size = input_len;
    } else {
      size_t input_len = strlen(input);
      size_t new_size = full_input_size + input_len + 2;
      char *new_full_input = realloc(full_input, new_size);
      if (new_full_input == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(full_input);
        free(input);
        break;
      }
      full_input = new_full_input;
      strcat(full_input, "\n");
      strcat(full_input, input);
      full_input_size += input_len + 1;
    }

    free(input);

    if (get_nesting_level(full_input) == 0) {
      if (strlen(full_input) > 0) {
        add_history(full_input);
      }

      interpret(full_input, "REPL");

      free(full_input);
      full_input = NULL;
      full_input_size = 0;
    }
  }
  
  if (full_input) {
    free(full_input);
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
