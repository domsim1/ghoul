#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "compiler.h"
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
} ArgTypes;

typedef enum {
  NATIVE_VARIADIC,
  NATIVE_NORMAL,
} NativeType;

static bool checkArgCount(int argCount, int expectedCount) {
  if (argCount != expectedCount) {
    runtimeError("Expected %d argument but got %d.", expectedCount, argCount);
    return false;
  }
  return true;
}

static bool checkArgs(int argCount, int expectedCount, Value *args,
                      NativeType type, ...) {
  if (type == NATIVE_NORMAL && !checkArgCount(argCount, expectedCount)) {
    return false;
  } else if (type == NATIVE_VARIADIC && argCount < expectedCount) {
    runtimeError("Expected %d argument but got %d.", expectedCount, argCount);
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
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_STRING:
      if (!IS_STRING(args[i])) {
        runtimeError("Expected argument %d to be a string.", i + 1);
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_LIST:
      if (!IS_LIST(args[i])) {
        runtimeError("Expected argument %d to be a list.", i + 1);
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_CLOSURE:
      if (!IS_CLOSURE(args[i])) {
        runtimeError("Expected argument %d to be a closure.", i + 1);
        va_end(expectedArgs);
        return false;
      }
      break;
    case ARG_KLASS:
      if (!IS_CLASS(args[i])) {
        runtimeError("Expected argument %d to be a class.", i + 1);
        va_end(expectedArgs);
        return false;
      }
      va_end(expectedArgs);
      break;
    case ARG_INSTANCE:
      if (!IS_INSTANCE(args[i])) {
        runtimeError("Expected argument %d to be an instance.", i + 1);
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

static void defineNative(const char *name, NativeFn function) {
  push(OBJ_VAL(copyString(name, (int)strlen(name), &vm.strings)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

static void defineGlobal(const char *name, Value value) {
  push(OBJ_VAL(copyString(name, (int)strlen(name), &vm.strings)));
  push(value);
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

static ObjInstance *defineInstance(ObjClass *klass, const char *name) {
  push(OBJ_VAL(copyString(name, (int)strlen(name), &vm.strings)));
  push(OBJ_VAL(klass));
  push(OBJ_VAL(newInstance(AS_CLASS(vm.stack[1]))));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[2]);
  ObjInstance *instance = AS_INSTANCE(vm.stack[2]);
  pop();
  pop();
  pop();
  return instance;
}

static ObjClass *defineKlass(const char *name) {
  push(OBJ_VAL(copyString(name, (int)strlen(name), &vm.strings)));
  push(OBJ_VAL(newClass(AS_STRING(vm.stack[0]))));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  ObjClass *klass = AS_CLASS(vm.stack[1]);
  pop();
  pop();
  return klass;
}

static void defineNativeKlassMethod(ObjClass *klass, const char *name,
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

static void defineNativeInstanceField(ObjInstance *instance, const char *name,
                                      Value value) {
  push(OBJ_VAL(instance));
  push(OBJ_VAL(copyString(name, (int)strlen(name), &vm.strings)));
  push(value);
  tableSet(&instance->fields, AS_STRING(vm.stack[1]), vm.stack[2]);
  pop();
  pop();
  pop();
}

static Value clockNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 0, args, NATIVE_NORMAL)) {
    return 0;
  };
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value exitNative(int argCount, Value *args) {
  if (argCount == 0) {
    exit(0);
  }
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_NUMBER)) {
    return 0;
  };
  exit(AS_NUMBER(args[0]));
}

static Value pushListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_VARIADIC, ARG_LIST, ARG_ANY)) {
    return 0;
  };
  ObjList *list = AS_LIST(args[0]);
  for (int i = 1; i < argCount; i++) {
    Value item = args[1];
    pushToList(list, item);
  }
  return NIL_VAL;
}

static Value popListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_LIST)) {
    return 0;
  };
  ObjList *list = AS_LIST(args[0]);
  Value value = list->items[list->count - 1];
  deleteFromList(list, list->count - 1);
  return value;
}

static Value countNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return 0;
  };
  if (IS_LIST(args[0])) {
    ObjList *list = AS_LIST(args[0]);
    double count = (double)list->count;
    return NUMBER_VAL(count);
  } else if (IS_STRING(args[0])) {
    ObjString *str = AS_STRING(args[0]);
    double count = (double)str->length;
    return NUMBER_VAL(count);
  }
  return NIL_VAL;
}

static Value deleteListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_LIST, ARG_NUMBER)) {
    return 0;
  }

  ObjList *list = AS_LIST(args[0]);
  int index = AS_NUMBER(args[1]);

  if (!isValidListIndex(list, index)) {
    runtimeError("List index out of range.");
    return 0;
  }

  deleteFromList(list, index);
  return NIL_VAL;
}

void registerNatives() {
  defineNative("clock", clockNative);
  defineNative("exit", exitNative);

  defineNative("push", pushListNative);
  defineNative("pop", popListNative);
  defineNative("remove", deleteListNative);
  defineNative("count", countNative);
}

void registerListNatives() {
  static bool isRegistered = false;
  if (isRegistered)
    return;

  ObjInstance *listInstance = defineInstance(defineKlass("List"), "List");
  defineNativeInstanceMethod(listInstance, "clock", clockNative);

  isRegistered = true;
}
