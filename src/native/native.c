#include "common_native.h"

bool checkArgCount(int argCount, int expectedCount) {
  if (argCount != expectedCount) {
    runtimeError("Expected %d argument but got %d.", expectedCount - 1,
                 argCount - 1);
    vm.shouldPanic = true;
    return false;
  }
  return true;
}

bool checkArgs(int argCount, int expectedCount, Value *args,
                      NativeType type, ...) {
  if (expectedCount == 0) {
    if (argCount != 1 && type != NATIVE_VARIADIC) {
      runtimeError("Expected no arguments but got %d.", argCount - 1);
      vm.shouldPanic = true;
      return false;
    }
    return true;
  }
  if (type == NATIVE_NORMAL && !checkArgCount(argCount, expectedCount)) {
    return false;
  } else if (type == NATIVE_VARIADIC && argCount < expectedCount) {
    runtimeError("Expected %d argument but got %d.", expectedCount - 1,
                 argCount - 1);
    vm.shouldPanic = true;
    return false;
  }

  va_list expectedArgs;
  va_start(expectedArgs, type);

  for (int i = 0; i < expectedCount; i++) {
    ArgTypes argType = va_arg(expectedArgs, ArgTypes);
    switch (argType) {
    case ARG_NUMBER:
      if (!IS_NUMBER(args[i])) {
        runtimeError("Expected argument %d to be a number.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_STRING:
      if (!IS_STRING(args[i])) {
        runtimeError("Expected argument %d to be a string.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_LIST:
      if (!IS_LIST(args[i])) {
        runtimeError("Expected argument %d to be a list.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_MAP:
      if (!IS_MAP(args[i])) {
        runtimeError("Expected argument %d to be a map.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_CLOSURE:
      if (!IS_CLOSURE(args[i])) {
        runtimeError("Expected argument %d to be a closure.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_KLASS:
      if (!IS_KLASS(args[i])) {
        runtimeError("Expected argument %d to be a class.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      va_end(expectedArgs);
      break;
    case ARG_INSTANCE:
      if (!IS_INSTANCE(args[i])) {
        runtimeError("Expected argument %d to be an instance.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_FILE:
      if (!IS_FILE(args[i])) {
        runtimeError("Expected argument %d to be an instance.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_BOOL:
      if (!IS_BOOL(args[i])) {
        runtimeError("Expected argument %d to be a bool.", i + 1);
        vm.shouldPanic = true;
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_ANY:
    default:
      continue;
    }
  }
  va_end(expectedArgs);
  return true;
}

void defineNative(const char *name, int len, NativeFn function) {
  push(OBJ_VAL(copyString(name, len, &vm.strings)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(peek(1)), peek(0));
  pop();
  pop();
}

ObjInstance *defineInstance(ObjKlass *klass, const char *name, int len) {
  push(OBJ_VAL(copyString(name, len, &vm.strings)));
  push(OBJ_VAL(klass));
  push(OBJ_VAL(newInstance(klass)));
  tableSet(&vm.globals, AS_STRING(peek(2)), peek(0));
  ObjInstance *instance = AS_INSTANCE(peek(0));
  pop();
  pop();
  pop();
  return instance;
}

ObjKlass *defineKlass(const char *name, int len, ObjType base) {
  push(OBJ_VAL(copyString(name, len, &vm.strings)));
  push(OBJ_VAL(newKlass(AS_STRING(peek(0)), base)));
  tableSet(&vm.globals, AS_STRING(peek(1)), peek(0));
  ObjKlass *klass = AS_KLASS(peek(0));
  pop();
  pop();
  return klass;
}

void defineNativeKlassMethod(ObjKlass *klass, const char *name, int len,
                                    NativeFn function) {
  push(OBJ_VAL(klass));
  push(OBJ_VAL(copyString(name, len, &vm.strings)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&klass->properties, AS_STRING(peek(1)), peek(0));
  pop();
  pop();
  pop();
}

void defineNativeInstanceMethod(ObjInstance *instance, const char *name,
                                       int len, NativeFn function) {
  push(OBJ_VAL(instance));
  push(OBJ_VAL(copyString(name, len, &vm.strings)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&instance->fields, AS_STRING(peek(1)), peek(0));
  pop();
  pop();
  pop();
}

void setNativeInstanceField(ObjInstance *instance, ObjString *string,
                                   Value value) {
  tableSet(&instance->fields, string, value);
}

Value readNativeInstanceField(ObjInstance *instance, const char *name, int len) {
  Value v;
  tableGet(&instance->fields, copyString(name, len, &vm.strings), &v); 
  return v;
}

void defineNativeInstanceField(ObjInstance *instance, const char *string,
                                      int len, Value value) {
  push(OBJ_VAL(copyString(string, len, &vm.strings)));
  setNativeInstanceField(instance, AS_STRING(peek(0)), value);
  pop();
}

static Value initFileNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING,
                 ARG_STRING)) {
    return NIL_VAL;
  };
  FILE *file;
  file = fopen(AS_CSTRING(args[1]), AS_CSTRING(args[2]));
  if (file == NULL) {
    runtimeError("Error opening file.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  ObjFile *file_;
  if (IS_FILE(args[0])) {
    file_ = AS_FILE(args[0]);
  } else if (IS_KLASS(args[0])) {
    file_ = newFile(AS_KLASS(args[0]));
  } else {
    file_ = newFile(vm.klass.file);
  }
  file_->file = file;
  return OBJ_VAL(file_);
}

static Value closeFileNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_FILE)) {
    return NIL_VAL;
  };
  ObjFile *file = AS_FILE(args[0]);
  fclose(file->file);
  file->file = NULL;
  return NIL_VAL;
}

static Value writeFileNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_FILE, ARG_STRING)) {
    return NIL_VAL;
  };
  ObjString *str = AS_STRING(args[1]);
  FILE *file = AS_FILE(args[0])->file;
  for (int i = 0; i < str->length; i++) {
    int status = fputc(str->chars[i], file);
    if (status == EOF) {
      runtimeError("Could not write to file!");
      vm.shouldPanic = true;
      return NIL_VAL;
    }
  }
  return NIL_VAL;
}

static Value readFileNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_FILE, ARG_STRING)) {
    return NIL_VAL;
  };
  FILE *file = AS_FILE(args[0])->file;
  ObjString *termStr = AS_STRING(args[1]);
  if (termStr->length > 1) {
    runtimeError("Read terminator should be empty for EOF or 1 char long.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  char term;
  if (termStr->length == 0) {
    term = EOF;
  } else {
    term = termStr->chars[0];
  }
  int count = 0;
  int capacity = 8;
  int oldCapacity = 0;
  char *str = NULL;
  str = GROW_ARRAY(char, str, oldCapacity, capacity);
  do {
    char c = fgetc(file);
    if (c == EOF) {
      break;
    }
    if (c == term) {
      break;
    }
    if (capacity < count + 1) {
      oldCapacity = capacity;
      capacity = GROW_CAPACITY(oldCapacity);
      str = GROW_ARRAY(char, str, oldCapacity, capacity);
    }
    str[count] = c;
    count++;
  } while (true);
  push(OBJ_VAL(copyString(str, count, &vm.strings)));
  FREE_ARRAY(char, str, capacity);
  return pop();
}

static Value initListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_VARIADIC, ARG_ANY)) {
    return NIL_VAL;
  };
  ObjList *list = NULL;
  if (IS_LIST(args[0])) {
    list = AS_LIST(args[0]);
  } else if (IS_KLASS(args[0])) {
    list = newList(AS_KLASS(args[0]));
  } else {
    runtimeError("Unexpect base for List init.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  push(OBJ_VAL(list));
  for (int i = 1; i < argCount; i++) {
    pushToList(list, args[i]);
  }
  return pop();
}

static Value pushListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_VARIADIC, ARG_LIST, ARG_ANY)) {
    return NIL_VAL;
  };
  ObjList *list = AS_LIST(args[0]);
  for (int i = 1; i < argCount; i++) {
    Value item = args[i];
    pushToList(list, item);
  }
  return NIL_VAL;
}

static Value popListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_LIST)) {
    return NIL_VAL;
  };
  ObjList *list = AS_LIST(args[0]);
  Value value = list->items[list->count - 1];
  deleteFromList(list, list->count - 1, list->count - 1);
  return value;
}

static Value lenListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_LIST)) {
    return NIL_VAL;
  };
  ObjList *list = AS_LIST(args[0]);
  double count = (double)list->count;
  return NUMBER_VAL(count);
}

static Value removeListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_LIST, ARG_NUMBER,
                 ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjList *list = AS_LIST(args[0]);
  int start = AS_NUMBER(args[1]);
  int end = AS_NUMBER(args[2]);
  if (!isValidListRange(list, start, end)) {
    runtimeError("List range index is out of range.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  deleteFromList(list, start, end);
  return NIL_VAL;
}

static Value joinListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_LIST, ARG_STRING)) {
    return NIL_VAL;
  }
  ObjList *list = AS_LIST(args[0]);
  ObjString *delimiter = AS_STRING(args[1]);
  int length = 0;

  for (int i = 0; i < list->count; i++) {
    Value item = list->items[i];
    if (IS_STRING(item)) {
      length += AS_STRING(item)->length;
    } else {
      runtimeError("Can only join a list of strings.");
      vm.shouldPanic = true;
      return NIL_VAL;
    }
    if (i < list->count - 1) {
      length += delimiter->length;
    }
  }

  char *result = ALLOCATE(char, length + 1);
  char *rpos = result;
  for (int i = 0; i < list->count; i++) {
    ObjString *str = AS_STRING(list->items[i]);
    memcpy(rpos, str->chars, str->length);
    rpos += str->length;
    if (i < list->count - 1) {
      memcpy(rpos, delimiter->chars, delimiter->length);
      rpos += delimiter->length;
    }
  }
  *rpos = '\0';
  return OBJ_VAL(takeString(result, length));
}

static Value initMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  };
  ObjMap *map = NULL;
  if (IS_MAP(args[0])) {
    map = AS_MAP(args[0]);
  } else if (IS_KLASS(args[0])) {
    map = newMap(AS_KLASS(args[0]));
  } else {
    runtimeError("Unexpect base for map init.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  return OBJ_VAL(map);
}

static Value keysMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_MAP)) {
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjList *list = newList(vm.klass.list);
  vm.keep = (Obj *)list;
  for (int i = 0; i < map->items.capacity; i++) {
    Entry entry = map->items.entries[i];
    if (entry.key == NULL) {
      continue;
    }
    pushToList(list, OBJ_VAL(entry.key));
  }
  vm.keep = NULL;
  return OBJ_VAL(list);
}

static Value valuesMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_MAP)) {
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjList *list = newList(vm.klass.list);
  vm.keep = (Obj *)list;
  for (int i = 0; i < map->items.capacity; i++) {
    Entry entry = map->items.entries[i];
    if (entry.key == NULL) {
      continue;
    }
    pushToList(list, entry.value);
  }
  vm.keep = NULL;
  return OBJ_VAL(list);
}

static Value pairsMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_MAP)) {
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjList *list = newList(vm.klass.list);
  push(OBJ_VAL(list));
  for (int i = 0; i < map->items.capacity; i++) {
    Entry entry = map->items.entries[i];
    if (entry.key == NULL) {
      continue;
    }
    ObjInstance *pair = newInstance(vm.klass.pair);
    push(OBJ_VAL(pair));
    defineNativeInstanceField(pair, "key", 3, OBJ_VAL(entry.key));
    defineNativeInstanceField(pair, "value", 5, entry.value);
    pushToList(list, pop());
  }
  return pop();
}

static Value hasKeyMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_MAP, ARG_STRING)) {
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjString *key = AS_STRING(args[1]);
  Value value;
  if (tableGet(&map->items, key, &value)) {
    return TRUE_VAL;
  }
  return FALSE_VAL;
}

static Value getMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_MAP, ARG_STRING)) {
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjString *key = AS_STRING(args[1]);
  Value value;
  if (tableGet(&map->items, key, &value)) {
    return value;
  }
  return NIL_VAL;
}

static Value setMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_VARIADIC, ARG_MAP, ARG_STRING,
                 ARG_ANY)) {
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjString *key = AS_STRING(args[1]);
  if (tableSet(&map->items, key, args[2])) {
    return args[2];
  }
  return NIL_VAL;
}

static Value deleteMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_MAP, ARG_STRING)) {
    return NIL_VAL;
  };
  ObjMap *map = AS_MAP(args[0]);
  ObjString *key = AS_STRING(args[1]);
  if (tableDelete(&map->items, key)) {
    return TRUE_VAL;
  }
  return FALSE_VAL;
}

static Value initStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_VARIADIC, ARG_ANY)) {
    return NIL_VAL;
  };
  ObjString *string = NULL;
  if (IS_STRING(args[0])) {
    string = AS_STRING(args[0]);
  } else if (IS_KLASS(args[0])) {
    if (argCount == 1) {
      string = copyEscString("", 0, &vm.strings, AS_KLASS(args[0]));
    } else if (argCount == 2) {
      if (IS_STRING(args[1])) {
        string = AS_STRING(args[1]);
        string->klass = AS_KLASS(args[0]);
      } else if (IS_NUMBER(args[1])) {
        int d_len = snprintf(NULL, 0, "%.15g", AS_NUMBER(args[1]));
        char d_str[d_len + 1];
        sprintf(d_str, "%.15g", AS_NUMBER(args[1]));
        string = copyEscString(d_str, d_len, &vm.strings, AS_KLASS(args[0]));

      } else {
        runtimeError("Expected argument to be string or number.");
        vm.shouldPanic = true;
        return NIL_VAL;
      }
    } else {
      runtimeError("Expected 1 argument but got %d.", argCount);
      vm.shouldPanic = true;
      return NIL_VAL;
    }
  } else {
    runtimeError("Unexpect base for String init.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }

  return OBJ_VAL(string);
}

static Value asNumberStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_VARIADIC, ARG_ANY)) {
    return NIL_VAL;
  };
  double num;
  int match = sscanf(AS_CSTRING(args[0]), "%lf", &num);
  if (match) {
    return NUMBER_VAL(num);
  }
  return NIL_VAL;
}

static Value toLowerCaseStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_STRING)) {
    return NIL_VAL;
  }
  
  ObjString *original = AS_STRING(args[0]);
  
  if (original->length == 0) {
    return args[0];
  }
  
  char *lowercased = ALLOCATE(char, original->length + 1);
  if (lowercased == NULL) {
    runtimeError("Memory allocation failed in lower.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  bool changed = false;
  for (int i = 0; i < original->length; i++) {
    unsigned char c = (unsigned char)original->chars[i];
    char lower_c = tolower(c);
    lowercased[i] = lower_c;
    if (lower_c != original->chars[i]) {
      changed = true;
    }
  }
  lowercased[original->length] = '\0';
  
  if (!changed) {
    FREE_ARRAY(char, lowercased, original->length + 1);
    return args[0];
  }
  
  push(OBJ_VAL(takeString(lowercased, original->length)));
  ObjString *result = AS_STRING(peek(0));
  result->klass = original->klass;
  return pop();
}

static Value toUpperCaseStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_STRING)) {
    return NIL_VAL;
  }
  
  ObjString *original = AS_STRING(args[0]);
  
  if (original->length == 0) {
    return args[0];
  }
  
  char *uppercased = ALLOCATE(char, original->length + 1);
  if (uppercased == NULL) {
    runtimeError("Memory allocation failed in upper.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  bool changed = false;
  for (int i = 0; i < original->length; i++) {
    unsigned char c = (unsigned char)original->chars[i];
    char upper_c = toupper(c);
    uppercased[i] = upper_c;
    if (upper_c != original->chars[i]) {
      changed = true;
    }
  }
  uppercased[original->length] = '\0';
  
  if (!changed) {
    FREE_ARRAY(char, uppercased, original->length + 1);
    return args[0];
  }
  
  push(OBJ_VAL(takeString(uppercased, original->length)));
  ObjString *result = AS_STRING(peek(0));
  result->klass = original->klass;
  return pop();
}

static Value indexOfStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING)) {
    return NIL_VAL;
  }
  
  ObjString *haystack_str = AS_STRING(args[0]);
  ObjString *needle_str = AS_STRING(args[1]);
  
  if (needle_str->length == 0) {
    return NUMBER_VAL(0);
  }
  
  if (haystack_str->length == 0 || needle_str->length > haystack_str->length) {
    return NUMBER_VAL(-1);
  }
  
  char *haystack = haystack_str->chars;
  char *needle = needle_str->chars;
  
  char *found = strstr(haystack, needle);
  if (found == NULL) {
    return NUMBER_VAL(-1);
  }
  
  ptrdiff_t index = found - haystack;
  if (index < 0 || index >= haystack_str->length) {
    return NUMBER_VAL(-1);
  }
  
  return NUMBER_VAL((double)index);
}

static Value lastIndexOfStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING)) {
    return NIL_VAL;
  }
  
  ObjString *haystack_str = AS_STRING(args[0]);
  ObjString *needle_str = AS_STRING(args[1]);
  
  if (needle_str->length == 0) {
    return NUMBER_VAL((double)haystack_str->length);
  }
  
  if (haystack_str->length == 0 || needle_str->length > haystack_str->length) {
    return NUMBER_VAL(-1);
  }
  
  char *haystack = haystack_str->chars;
  char *needle = needle_str->chars;
  char *last_found = NULL;
  char *current = haystack;
  
  while ((current = strstr(current, needle)) != NULL) {
    last_found = current;
    current += needle_str->length;
    if (current > haystack + haystack_str->length) {
      break;
    }
  }
  
  if (last_found == NULL) {
    return NUMBER_VAL(-1);
  }
  
  ptrdiff_t index = last_found - haystack;
  if (index < 0 || index >= haystack_str->length) {
    return NUMBER_VAL(-1);
  }
  
  return NUMBER_VAL((double)index);
}

static Value startsWithStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING)) {
    return NIL_VAL;
  }
  
  ObjString *string_str = AS_STRING(args[0]);
  ObjString *prefix_str = AS_STRING(args[1]);
  
  if (prefix_str->length == 0) {
    return TRUE_VAL;
  }
  
  if (string_str->length < prefix_str->length) {
    return FALSE_VAL;
  }
  
  return (memcmp(string_str->chars, prefix_str->chars, prefix_str->length) == 0) ? TRUE_VAL : FALSE_VAL;
}

static Value endsWithStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING)) {
    return NIL_VAL;
  }
  
  ObjString *string_str = AS_STRING(args[0]);
  ObjString *suffix_str = AS_STRING(args[1]);
  
  if (suffix_str->length == 0) {
    return TRUE_VAL;
  }
  
  if (string_str->length < suffix_str->length) {
    return FALSE_VAL;
  }
  
  char *suffix_start = string_str->chars + string_str->length - suffix_str->length;
  return (memcmp(suffix_start, suffix_str->chars, suffix_str->length) == 0) ? TRUE_VAL : FALSE_VAL;
}

static Value trimStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_STRING)) {
    return NIL_VAL;
  }
  
  ObjString *original = AS_STRING(args[0]);
  
  if (original->length == 0) {
    return args[0];
  }
  
  char *start = original->chars;
  char *end = original->chars + original->length - 1;
  
  while (start <= end && isspace((unsigned char)*start)) {
    start++;
  }
  
  while (end >= start && isspace((unsigned char)*end)) {
    end--;
  }
  
  int new_length = (int)(end - start + 1);
  if (new_length <= 0) {
    push(OBJ_VAL(copyString("", 0, &vm.strings)));
    ObjString *result = AS_STRING(peek(0));
    result->klass = original->klass;
    return pop();
  }
  
  if (new_length == original->length) {
    return args[0];
  }
  
  if (new_length > original->length) {
    runtimeError("Invalid length calculation in trim.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  char *trimmed = ALLOCATE(char, new_length + 1);
  if (trimmed == NULL) {
    runtimeError("Memory allocation failed in trim.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  memcpy(trimmed, start, new_length);
  trimmed[new_length] = '\0';
  
  push(OBJ_VAL(takeString(trimmed, new_length)));
  ObjString *result = AS_STRING(peek(0));
  result->klass = original->klass;
  return pop();
}

static Value substringStringNative(int argCount, Value *args) {
  if (argCount < 2 || argCount > 3) {
    runtimeError("substring() takes 1 or 2 arguments but got %d.", argCount - 1);
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  if (!IS_STRING(args[0]) || !IS_NUMBER(args[1])) {
    runtimeError("substring() expects string and number arguments.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  if (argCount == 3 && !IS_NUMBER(args[2])) {
    runtimeError("substring() end index must be a number.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  ObjString *original = AS_STRING(args[0]);
  double start_d = AS_NUMBER(args[1]);
  double end_d = (argCount == 3) ? AS_NUMBER(args[2]) : (double)original->length;
  
  if (start_d < INT_MIN || start_d > INT_MAX || end_d < INT_MIN || end_d > INT_MAX) {
    runtimeError("Index out of range in substring.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  int start = (int)start_d;
  int end = (int)end_d;
  
  if (start < 0) start = 0;
  if (end > original->length) end = original->length;
  if (start > end) start = end;
  if (start > original->length) start = original->length;
  
  int new_length = end - start;
  if (new_length <= 0) {
    push(OBJ_VAL(copyString("", 0, &vm.strings)));
    ObjString *result = AS_STRING(peek(0));
    result->klass = original->klass;
    return pop();
  }
  
  if (new_length == original->length && start == 0) {
    return args[0];
  }
  
  if (start + new_length > original->length) {
    runtimeError("Substring bounds exceed string length.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  char *substring = ALLOCATE(char, new_length + 1);
  if (substring == NULL) {
    runtimeError("Memory allocation failed in substring.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  memcpy(substring, original->chars + start, new_length);
  substring[new_length] = '\0';
  
  push(OBJ_VAL(takeString(substring, new_length)));
  ObjString *result = AS_STRING(peek(0));
  result->klass = original->klass;
  return pop();
}

static Value replaceStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING, ARG_STRING)) {
    return NIL_VAL;
  }
  
  ObjString *original = AS_STRING(args[0]);
  char *search = AS_CSTRING(args[1]);
  char *replace = AS_CSTRING(args[2]);
  
  int search_len = strlen(search);
  int replace_len = strlen(replace);
  
  if (search_len == 0) {
    return args[0];
  }
  
  char *found = strstr(original->chars, search);
  if (found == NULL) {
    return args[0];
  }
  
  int prefix_len = (int)(found - original->chars);
  int suffix_len = original->length - prefix_len - search_len;
  
  if (prefix_len < 0 || suffix_len < 0 || prefix_len > original->length) {
    runtimeError("Invalid string bounds in replace.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  long long new_length_ll = (long long)prefix_len + replace_len + suffix_len;
  if (new_length_ll < 0 || new_length_ll > INT_MAX) {
    runtimeError("Result string too large in replace.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  int new_length = (int)new_length_ll;
  char *result = ALLOCATE(char, new_length + 1);
  if (result == NULL) {
    runtimeError("Memory allocation failed in replace.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  memcpy(result, original->chars, prefix_len);
  memcpy(result + prefix_len, replace, replace_len);
  memcpy(result + prefix_len + replace_len, found + search_len, suffix_len);
  result[new_length] = '\0';
  
  push(OBJ_VAL(takeString(result, new_length)));
  ObjString *result_str = AS_STRING(peek(0));
  result_str->klass = original->klass;
  return pop();
}

static Value replaceAllStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING, ARG_STRING)) {
    return NIL_VAL;
  }
  
  ObjString *original = AS_STRING(args[0]);
  char *search = AS_CSTRING(args[1]);
  char *replace = AS_CSTRING(args[2]);
  
  int search_len = strlen(search);
  int replace_len = strlen(replace);
  
  if (search_len == 0) {
    return args[0];
  }
  
  int count = 0;
  char *temp = original->chars;
  while ((temp = strstr(temp, search)) != NULL) {
    count++;
    temp += search_len;
    if (count > 10000) {
      runtimeError("Too many replacements in replace_all.");
      vm.shouldPanic = true;
      return NIL_VAL;
    }
  }
  
  if (count == 0) {
    return args[0];
  }
  
  long long new_length_ll = (long long)original->length + (long long)count * (replace_len - search_len);
  if (new_length_ll < 0 || new_length_ll > INT_MAX) {
    runtimeError("Result string too large in replace_all.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  int new_length = (int)new_length_ll;
  char *result = ALLOCATE(char, new_length + 1);
  
  char *src = original->chars;
  char *dst = result;
  char *result_end = result + new_length;
  
  while (*src && dst < result_end) {
    char *found = strstr(src, search);
    if (found == src) {
      if (dst + replace_len > result_end) {
        FREE_ARRAY(char, result, new_length + 1);
        runtimeError("Buffer overflow in replace_all.");
        vm.shouldPanic = true;
        return NIL_VAL;
      }
      memcpy(dst, replace, replace_len);
      dst += replace_len;
      src += search_len;
    } else {
      if (dst >= result_end) {
        FREE_ARRAY(char, result, new_length + 1);
        runtimeError("Buffer overflow in replace_all.");
        vm.shouldPanic = true;
        return NIL_VAL;
      }
      *dst++ = *src++;
    }
  }
  
  if (dst > result_end) {
    FREE_ARRAY(char, result, new_length + 1);
    runtimeError("Buffer overflow in replace_all.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  *dst = '\0';
  
  push(OBJ_VAL(takeString(result, (int)(dst - result))));
  ObjString *result_str = AS_STRING(peek(0));
  result_str->klass = original->klass;
  return pop();
}

static Value lenStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_STRING)) {
    return NIL_VAL;
  };
  ObjString *string = AS_STRING(args[0]);
  double count = (double)string->length;
  return NUMBER_VAL(count);
}

static Value containsStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING)) {
    return NIL_VAL;
  };

  char *string = AS_CSTRING(args[0]);
  char *term = AS_CSTRING(args[1]);

  return (strstr(string, term) != NULL) ? TRUE_VAL : FALSE_VAL;
}

static Value splitStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING)) {
    return NIL_VAL;
  };
  ObjString *string = AS_STRING(args[0]);
  char *term = AS_CSTRING(args[1]);
  ObjList *list = newList(vm.klass.list);
  push(OBJ_VAL(list));
  char *lastSplit = string->chars;
  ObjString *str = NULL;

  while (true) {
    char *cp = strstr(lastSplit, term);
    if (cp == NULL) {
      str = copyString(lastSplit,
                       (int)(&string->chars[string->length] - lastSplit),
                       &vm.strings);
      pushToList(list, OBJ_VAL(str));
      return pop();
    }

    str = copyString(lastSplit, (int)(cp - lastSplit), &vm.strings);
    pushToList(list, OBJ_VAL(str));
    lastSplit = cp + 1;
  }
}


static Value initErrorNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    return NIL_VAL;
  };
  ObjInstance *err = NULL;
  if (IS_KLASS(args[0])) {
    err = newInstance(AS_KLASS(args[0]));
  } else if (IS_INSTANCE(args[0])) {
    err = AS_INSTANCE(args[0]);
  } else {
    runtimeError("Could not init error.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  push(OBJ_VAL(err));
  setNativeInstanceField(err, vm.string.message, args[1]);
  setNativeInstanceField(err, vm.string.isError, TRUE_VAL);
  return pop();
}


static ObjKlass *createFileClass() {
  ObjKlass *fileKlass = defineKlass("File", 4, OBJ_FILE);
  defineNativeKlassMethod(fileKlass, "init", 4, initFileNative);
  defineNativeKlassMethod(fileKlass, "close", 5, closeFileNative);
  defineNativeKlassMethod(fileKlass, "write", 5, writeFileNative);
  defineNativeKlassMethod(fileKlass, "read", 4, readFileNative);

  return fileKlass;
}

static ObjKlass *createListClass() {
  ObjKlass *listKlass = defineKlass("List", 4, OBJ_LIST);
  defineNativeKlassMethod(listKlass, "init", 4, initListNative);
  defineNativeKlassMethod(listKlass, "push", 4, pushListNative);
  defineNativeKlassMethod(listKlass, "pop", 3, popListNative);
  defineNativeKlassMethod(listKlass, "len", 3, lenListNative);
  defineNativeKlassMethod(listKlass, "remove", 6, removeListNative);
  defineNativeKlassMethod(listKlass, "join", 4, joinListNative);

  return listKlass;
}

static ObjKlass *createMapClass() {
  ObjKlass *mapKlass = defineKlass("Map", 3, OBJ_MAP);
  defineNativeKlassMethod(mapKlass, "init", 4, initMapNative);
  defineNativeKlassMethod(mapKlass, "keys", 4, keysMapNative);
  defineNativeKlassMethod(mapKlass, "values", 6, valuesMapNative);
  defineNativeKlassMethod(mapKlass, "pairs", 5, pairsMapNative);
  defineNativeKlassMethod(mapKlass, "has", 3, hasKeyMapNative);
  defineNativeKlassMethod(mapKlass, "get", 3, getMapNative);
  defineNativeKlassMethod(mapKlass, "set", 3, setMapNative);
  defineNativeKlassMethod(mapKlass, "delete", 6, deleteMapNative);

  return mapKlass;
}

static ObjKlass *createPairClass() {
  ObjKlass *pairKlass = defineKlass("Pair", 4, OBJ_INSTANCE);

  return pairKlass;
}

static ObjKlass *createStringClass() {
  ObjKlass *stringKlass = defineKlass("String", 6, OBJ_STRING);
  defineNativeKlassMethod(stringKlass, "init", 4, initStringNative);
  defineNativeKlassMethod(stringKlass, "len", 3, lenStringNative);
  defineNativeKlassMethod(stringKlass, "contains", 8, containsStringNative);
  defineNativeKlassMethod(stringKlass, "split", 5, splitStringNative);
  defineNativeKlassMethod(stringKlass, "asnum", 5, asNumberStringNative);
  defineNativeKlassMethod(stringKlass, "lower", 5, toLowerCaseStringNative);
  defineNativeKlassMethod(stringKlass, "upper", 5, toUpperCaseStringNative);
  defineNativeKlassMethod(stringKlass, "index_of", 8, indexOfStringNative);
  defineNativeKlassMethod(stringKlass, "last_index_of", 13, lastIndexOfStringNative);
  defineNativeKlassMethod(stringKlass, "starts_with", 11, startsWithStringNative);
  defineNativeKlassMethod(stringKlass, "ends_with", 9, endsWithStringNative);
  defineNativeKlassMethod(stringKlass, "trim", 4, trimStringNative);
  defineNativeKlassMethod(stringKlass, "substring", 9, substringStringNative);
  defineNativeKlassMethod(stringKlass, "replace", 7, replaceStringNative);
  defineNativeKlassMethod(stringKlass, "replace_all", 11, replaceAllStringNative);

  return stringKlass;
}

static ObjKlass *createErrorClass() {
  ObjKlass *errKlass = defineKlass("Error", 5, OBJ_INSTANCE);
  defineNativeKlassMethod(errKlass, "init", 4, initErrorNative);

  return errKlass;
}

void registerBuiltInKlasses() {
  vm.klass.list = createListClass();
  vm.klass.file = createFileClass();
  vm.klass.string = createStringClass();
  vm.klass.error = createErrorClass();
  vm.klass.map = createMapClass();
  vm.klass.pair = createPairClass();
}
