#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <readline/readline.h>
#include <readline/history.h>

#ifdef _WIN32
  #include <windows.h>
  #include <libgen.h>
  #ifndef PATH_MAX
    #define PATH_MAX MAX_PATH
  #endif
#elif defined(__APPLE__)
  #include <mach-o/dyld.h>
  #include <libgen.h>
  #include <limits.h>
  #include <unistd.h>
#else
  #include <unistd.h>
  #include <libgen.h>
  #include <limits.h>
#endif

#include "vm.h"

static char actualpath[PATH_MAX + 1];

static void setupUTF8Support() {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#else
  setlocale(LC_ALL, "");
  setlocale(LC_CTYPE, "en_US.UTF-8");
#endif
}

static char* portable_dirname(char* path) {
  char* last_slash = NULL;
  char* p = path;
  
  if (!path || !*path) {
    return ".";
  }
  
  while (*p) {
    if (*p == '/' || *p == '\\') {
      last_slash = p;
    }
    p++;
  }
  
  if (!last_slash) {
    return ".";
  }
  
  if (last_slash == path) {
    *(last_slash + 1) = '\0';
    return path;
  }
  
  *last_slash = '\0';
  return path;
}

static char* getExecutableDir(void) {
  static char execDir[PATH_MAX + 1];
  static int initialized = 0;
  
  if (initialized) {
    return execDir;
  }

  char path[PATH_MAX + 1] = {0};

#ifdef _WIN32
  DWORD len = GetModuleFileNameA(NULL, path, sizeof(path) - 1);
  if (len == 0) {
    fprintf(stderr, "Failed to get executable path on Windows (Error: %lu)\n", GetLastError());
    return NULL;
  }
  if (len >= sizeof(path) - 1) {
    fprintf(stderr, "Executable path too long on Windows\n");
    return NULL;
  }
  path[len] = '\0';

#elif defined(__APPLE__)
  uint32_t size = sizeof(path) - 1;
  if (_NSGetExecutablePath(path, &size) != 0) {
    fprintf(stderr, "Executable path too long on macOS (required size: %u)\n", size);
    return NULL;
  }
  
  char resolved[PATH_MAX + 1];
  if (realpath(path, resolved) != NULL) {
    if (strlen(resolved) < sizeof(path)) {
      strcpy(path, resolved);
    } else {
      fprintf(stderr, "Resolved path too long on macOS\n");
      return NULL;
    }
  }

#else
  ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
  if (len == -1) {
    fprintf(stderr, "Failed to get executable path on Linux\n");
    return NULL;
  }
  if (len == 0) {
    fprintf(stderr, "Empty executable path on Linux\n");
    return NULL;
  }
  if (len >= (ssize_t)(sizeof(path) - 1)) {
    fprintf(stderr, "Executable path too long on Linux\n");
    return NULL;
  }
  path[len] = '\0';
#endif

  char temp[PATH_MAX + 1];
  if (strlen(path) >= sizeof(temp)) {
    fprintf(stderr, "Path too long for processing\n");
    return NULL;
  }
  strcpy(temp, path);

  char* dir = portable_dirname(temp);
  if (!dir) {
    fprintf(stderr, "Failed to get directory name\n");
    return NULL;
  }
  
  if (strlen(dir) >= sizeof(execDir)) {
    fprintf(stderr, "Directory path too long for buffer\n");
    return NULL;
  }
  strcpy(execDir, dir);

  initialized = 1;
  return execDir;
}

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
      if (*p == string_char && (p == input || *(p - 1) != '\\')) {
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

static int file_exists(const char* path) {
  FILE* file = fopen(path, "r");
  if (file) {
    fclose(file);
    return 1;
  }
  return 0;
}

static void loadStd() {
  static char stdPath[PATH_MAX + 1];
  int found = 0;

  const char* search_paths[] = {
    NULL,                                    // Will be set to executable dir
#ifdef _WIN32
    "C:\\Program Files\\ghoul\\std\\std.ghoul",
    "C:\\ghoul\\std\\std.ghoul",
#else
    "/usr/local/share/ghoul/std/std.ghoul",  // System install location
    "/usr/share/ghoul/std/std.ghoul",        // Alternative system location
    "/opt/ghoul/std/std.ghoul",              // Optional software location
#endif
    "std/std.ghoul",                         // Current directory fallback
    "./std/std.ghoul",                       // Explicit current directory
    NULL
  };

  // Try executable directory first
  char *execDir = getExecutableDir();
  if (execDir != NULL) {
    int result = snprintf(stdPath, sizeof(stdPath), "%s%sstd%sstd.ghoul",
                          execDir,
#ifdef _WIN32
                          "\\", "\\"
#else
                          "/", "/"
#endif
    );

    if (result > 0 && result < PATH_MAX && file_exists(stdPath)) {
      found = 1;
    }
  }

  // If not found in executable directory, try other locations
  if (!found) {
    for (int i = 1; search_paths[i] != NULL; i++) {
      if (file_exists(search_paths[i])) {
        strncpy(stdPath, search_paths[i], sizeof(stdPath) - 1);
        stdPath[sizeof(stdPath) - 1] = '\0';
        found = 1;
        break;
      }
    }
  }

  if (!found) {
    fprintf(stderr, "Error: Could not find standard library (std.ghoul).\n");
    fprintf(stderr, "Searched in:\n");
    if (execDir) {
      fprintf(stderr, "  %s%sstd%sstd.ghoul\n", execDir,
#ifdef _WIN32
              "\\", "\\"
#else
              "/", "/"
#endif
      );
    }
    for (int i = 1; search_paths[i] != NULL; i++) {
      fprintf(stderr, "  %s\n", search_paths[i]);
    }
    fprintf(stderr, "Please ensure ghoul is properly installed or run from the source directory.\n");
    exit(74);
  }

  runFile(stdPath);
}

int main(int argc, const char *argv[]) {
  setupUTF8Support();
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