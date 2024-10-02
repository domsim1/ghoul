#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

typedef enum {
  ARG_ANY,
  ARG_NUMBER,
  ARG_STRING,
  ARG_LIST,
  ARG_CLOSURE,
  ARG_KLASS,
  ARG_INSTANCE,
  ARG_FILE,
} ArgTypes;

typedef enum {
  NATIVE_VARIADIC,
  NATIVE_NORMAL,
} NativeType;

static bool checkArgCount(int argCount, int expectedCount) {
  if (argCount != expectedCount) {
    runtimeError("Expected %d argument but got %d.", expectedCount - 1,
                 argCount - 1);
    vm.shouldPanic = true;
    return false;
  }
  return true;
}

static bool checkArgs(int argCount, int expectedCount, Value *args,
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
    case ARG_ANY:
    default:
      continue;
    }
  }
  va_end(expectedArgs);
  return true;
}

static void defineNative(const char *name, NativeFn function) {
  push(OBJ_VAL(copyString(name, (int)strlen(name), &vm.strings)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

static ObjInstance *defineInstance(ObjKlass *klass, const char *name) {
  push(OBJ_VAL(copyString(name, (int)strlen(name), &vm.strings)));
  push(OBJ_VAL(klass));
  push(OBJ_VAL(newInstance(AS_KLASS(vm.stack[1]))));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[2]);
  ObjInstance *instance = AS_INSTANCE(vm.stack[2]);
  pop();
  pop();
  pop();
  return instance;
}

static ObjKlass *defineKlass(const char *name, ObjType base) {
  push(OBJ_VAL(copyString(name, (int)strlen(name), &vm.strings)));
  push(OBJ_VAL(newKlass(AS_STRING(vm.stack[0]), base)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  ObjKlass *klass = AS_KLASS(vm.stack[1]);
  pop();
  pop();
  return klass;
}

static void defineNativeKlassMethod(ObjKlass *klass, const char *name,
                                    NativeFn function) {
  push(OBJ_VAL(klass));
  push(OBJ_VAL(copyString(name, (int)strlen(name), &vm.strings)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&klass->methods, AS_STRING(vm.stack[1]), vm.stack[2]);
  pop();
  pop();
  pop();
}

static void defineNativeInstanceMethod(ObjInstance *instance, const char *name,
                                       NativeFn function) {
  push(OBJ_VAL(instance));
  push(OBJ_VAL(copyString(name, (int)strlen(name), &vm.strings)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&instance->fields, AS_STRING(vm.stack[1]), vm.stack[2]);
  pop();
  pop();
  pop();
}

static void setNativeInstanceField(ObjInstance *instance, ObjString *string,
                                   Value value) {
  tableSet(&instance->fields, string, value);
}

static Value clockNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 0, args, NATIVE_NORMAL)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value exitNative(int argCount, Value *args) {
  if (argCount == 1) {
    exit(0);
  }
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  exit(AS_NUMBER(args[1]));
}

static Value panicNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  vm.shouldPanic = true;
  if (IS_INSTANCE(args[1])) {
    ObjInstance *err = AS_INSTANCE(args[1]);
    Value isError;
    if (tableGet(&err->fields, vm.string.isError, &isError)) {
      if (IS_BOOL(isError) && AS_BOOL(isError)) {
        Value message;
        if (tableGet(&err->fields, vm.string.message, &message) &&
            IS_STRING(message)) {
          fprintf(stderr, "%s: ", err->klass->name->chars);
          runtimeError(AS_CSTRING(message));
          return NIL_VAL;
        }
      }
    }
  } else if (IS_STRING(args[1])) {
    runtimeError(AS_CSTRING(args[1]));
    return NIL_VAL;
  }
  runtimeError("Panic with unepxected value!");
  return NIL_VAL;
}

static Value initFileNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING,
                 ARG_STRING)) {
    vm.shouldPanic = true;
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
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjFile *file = AS_FILE(args[0]);
  fclose(file->file);
  file->file = NULL;
  return NIL_VAL;
}

static Value writeFileNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_FILE, ARG_STRING)) {
    vm.shouldPanic = true;
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
    vm.shouldPanic = true;
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
    vm.shouldPanic = true;
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
    vm.shouldPanic = true;
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
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjList *list = AS_LIST(args[0]);
  Value value = list->items[list->count - 1];
  deleteFromList(list, list->count - 1, list->count - 1);
  return value;
}

static Value lenListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_LIST)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjList *list = AS_LIST(args[0]);
  double count = (double)list->count;
  return NUMBER_VAL(count);
}

static Value removeListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_LIST, ARG_NUMBER,
                 ARG_NUMBER)) {
    vm.shouldPanic = true;
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
  if (!checkArgs(argCount, 2, args, NATIVE_VARIADIC, ARG_LIST, ARG_LIST)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  ObjList *list = newList(vm.klass.list);
  push(OBJ_VAL(list));
  for (int arg = 0; arg < argCount; arg++) {
    ObjList *listb = AS_LIST(args[arg]);
    for (int i = 0; i < listb->count; i++) {
      pushToList(list, listb->items[i]);
    }
  }
  return OBJ_VAL(pop());
}

static Value initStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_VARIADIC, ARG_ANY)) {
    vm.shouldPanic = true;
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
        int d_len = snprintf(NULL, 0, "%g", AS_NUMBER(args[1]));
        char d_str[d_len + 1];
        sprintf(d_str, "%g", AS_NUMBER(args[1]));
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

static Value lenStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  ObjString *string = AS_STRING(args[0]);
  double count = (double)string->length;
  return NUMBER_VAL(count);
}

static Value containsStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };

  char *string = AS_CSTRING(args[0]);
  char *term = AS_CSTRING(args[1]);

  return (strstr(string, term) != NULL) ? TRUE_VAL : FALSE_VAL;
}

static Value splitStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_STRING, ARG_STRING)) {
    vm.shouldPanic = true;
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

static Value isInstOfNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE,
                 ARG_KLASS)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return AS_INSTANCE(args[1])->klass == AS_KLASS(args[2]) ? TRUE_VAL
                                                          : FALSE_VAL;
}

static Value isErrorNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };

  if (IS_INSTANCE(args[1])) {
    ObjInstance *err = AS_INSTANCE(args[1]);
    Value isError;
    if (tableGet(&err->fields, vm.string.isError, &isError)) {
      if (IS_BOOL(isError) && AS_BOOL(isError)) {
        return TRUE_VAL;
      }
    }
  }
  return FALSE_VAL;
}

static Value initErrorNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    vm.shouldPanic = true;
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
  ObjKlass *fileKlass = defineKlass("File", OBJ_FILE);
  defineNativeKlassMethod(fileKlass, "init", initFileNative);
  defineNativeKlassMethod(fileKlass, "close", closeFileNative);
  defineNativeKlassMethod(fileKlass, "write", writeFileNative);
  defineNativeKlassMethod(fileKlass, "read", readFileNative);

  return fileKlass;
}

static ObjKlass *createListClass() {
  ObjKlass *listKlass = defineKlass("List", OBJ_LIST);
  defineNativeKlassMethod(listKlass, "init", initListNative);
  defineNativeKlassMethod(listKlass, "push", pushListNative);
  defineNativeKlassMethod(listKlass, "pop", popListNative);
  defineNativeKlassMethod(listKlass, "len", lenListNative);
  defineNativeKlassMethod(listKlass, "remove", removeListNative);
  defineNativeKlassMethod(listKlass, "join", joinListNative);

  return listKlass;
}

static ObjKlass *createStringClass() {
  ObjKlass *stringKlass = defineKlass("String", OBJ_STRING);
  defineNativeKlassMethod(stringKlass, "init", initStringNative);
  defineNativeKlassMethod(stringKlass, "len", lenStringNative);
  defineNativeKlassMethod(stringKlass, "contains", containsStringNative);
  defineNativeKlassMethod(stringKlass, "split", splitStringNative);

  return stringKlass;
}

static ObjKlass *createErrorClass() {
  ObjKlass *errKlass = defineKlass("Error", OBJ_INSTANCE);
  defineNativeKlassMethod(errKlass, "init", initErrorNative);

  return errKlass;
}

void registerNatives() {
  defineNative("tick", clockNative);
  defineNative("exit", exitNative);
  defineNative("open", initFileNative);
  defineNative("iserr", isErrorNative);
  defineNative("instof", isInstOfNative);
  defineNative("panic", panicNative);
}

void registerBuiltInKlasses() {
  vm.klass.list = createListClass();
  vm.klass.file = createFileClass();
  vm.klass.string = createStringClass();
  vm.klass.error = createErrorClass();
}

void registerListNatives() {
  static bool isRegistered = false;
  if (isRegistered)
    return;

  ObjInstance *listInstance =
      defineInstance(defineKlass("Lists", OBJ_LIST), "Lists");
  defineNativeInstanceMethod(listInstance, "clock", clockNative);

  isRegistered = true;
}
