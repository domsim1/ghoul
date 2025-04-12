#include "common_native.h"

static Value tickNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 0, args, NATIVE_NORMAL)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value sleepNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  int sleepTime = AS_NUMBER(args[1]) * 1000000;
  return NUMBER_VAL(usleep(sleepTime));
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

static Value promptNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  char *input = readline(AS_CSTRING(args[1]));
  push(OBJ_VAL(copyString(input, strlen(input), &vm.strings)));
  free(input);
  return pop();
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

static Value isNumberNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_NUMBER(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isStringNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_STRING(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isInstNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_INSTANCE(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isKlassNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_KLASS(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_LIST(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isMapNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_MAP(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isBoolNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_BOOL(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isNilNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_NIL(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isFuncNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };
  return IS_CLOSURE(args[1]) ? TRUE_VAL : FALSE_VAL;
}

static Value isInstOfNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_ANY, ARG_ANY)) {
    vm.shouldPanic = true;
    return NIL_VAL;
  };

  if (!IS_INSTANCE(args[1])) {
    return FALSE_VAL;
  }

  if (IS_KLASS(args[2])) {
    return AS_INSTANCE(args[1])->klass == AS_KLASS(args[2]) ? TRUE_VAL
                                                            : FALSE_VAL;
  }
  if (IS_INSTANCE(args[2])) {
    return AS_INSTANCE(args[1])->klass == AS_INSTANCE(args[2])->klass
               ? TRUE_VAL
               : FALSE_VAL;
  }

  return FALSE_VAL;
}

void registerNatives() {
  defineNative("tick", 4, tickNative);
  defineNative("sleep", 5, sleepNative);
  defineNative("exit", 4, exitNative);
  defineNative("iserr", 5, isErrorNative);
  defineNative("instof", 6, isInstOfNative);
  defineNative("panic", 5, panicNative);
  defineNative("isnum", 5, isNumberNative);
  defineNative("isstr", 5, isStringNative);
  defineNative("isinst", 6, isInstNative);
  defineNative("isclass", 7, isKlassNative);
  defineNative("islist", 6, isListNative);
  defineNative("ismap", 5, isMapNative);
  defineNative("isbool", 6, isBoolNative);
  defineNative("isnil", 5, isNilNative);
  defineNative("isfn", 4, isFuncNative);
  defineNative("prompt", 6, promptNative);
}
