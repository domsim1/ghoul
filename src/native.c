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
  if (expectedCount == 0) {
    if (argCount != 1 && type != NATIVE_VARIADIC) {
      runtimeError("Expected no arguments but got %d.", argCount - 1);
      return false;
    }
    return true;
  }
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
  if (argCount == 1) {
    exit(0);
  }
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return 0;
  };
  exit(AS_NUMBER(args[1]));
}

static Value pushListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_VARIADIC, ARG_LIST, ARG_ANY)) {
    return 0;
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
    return 0;
  };
  ObjList *list = AS_LIST(args[0]);
  Value value = list->items[list->count - 1];
  deleteFromList(list, list->count - 1, list->count - 1);
  return value;
}

static Value initListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 0, args, NATIVE_VARIADIC)) {
    return 0;
  };
  ObjList *list = newList(vm.listKlass);
  push(OBJ_VAL(list));
  for (int i = 1; i < argCount; i++) {
    pushToList(list, args[i]);
  }
  return pop();
}

static Value lenListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_LIST)) {
    return 0;
  };
  ObjList *list = AS_LIST(args[0]);
  double count = (double)list->count;
  return NUMBER_VAL(count);
}

static Value removeListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_LIST, ARG_NUMBER,
                 ARG_NUMBER)) {
    return 0;
  }
  ObjList *list = AS_LIST(args[0]);
  int start = AS_NUMBER(args[1]);
  int end = AS_NUMBER(args[2]);
  if (!isValidListRange(list, start, end)) {
    runtimeError("List range index is out of range.");
    return 0;
  }
  deleteFromList(list, start, end);
  return NIL_VAL;
}

static Value joinListNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_VARIADIC, ARG_LIST, ARG_LIST)) {
    return 0;
  }
  ObjList *list = newList(vm.listKlass);
  push(OBJ_VAL(list));
  for (int arg = 0; arg < argCount; arg++) {
    ObjList *listb = AS_LIST(args[arg]);
    for (int i = 0; i < listb->count; i++) {
      pushToList(list, listb->items[i]);
    }
  }
  return OBJ_VAL(list);
}

void registerNatives() {
  defineNative("tick", clockNative);
  defineNative("exit", exitNative);
}

ObjClass *createListClass() {
  ObjClass *listKlass = defineKlass("List");
  defineNativeKlassMethod(listKlass, "init", initListNative);
  defineNativeKlassMethod(listKlass, "push", pushListNative);
  defineNativeKlassMethod(listKlass, "pop", popListNative);
  defineNativeKlassMethod(listKlass, "len", lenListNative);
  defineNativeKlassMethod(listKlass, "remove", removeListNative);
  defineNativeKlassMethod(listKlass, "join", joinListNative);
  return listKlass;
}

void registerListNatives() {
  static bool isRegistered = false;
  if (isRegistered)
    return;

  ObjInstance *listInstance = defineInstance(defineKlass("Lists"), "Lists");
  defineNativeInstanceMethod(listInstance, "clock", clockNative);

  isRegistered = true;
}
