#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "object.h"
#include "value.h"
#include "vm.h"

static bool checkArgCount(int argCount, int expectedCount) {
  if (argCount != expectedCount) {
    runtimeError("Expected %d argument but got %d.", expectedCount, argCount);
    return false;
  }
  return true;
}

static void defineNative(const char *name, NativeFn function) {
  push(OBJ_VAL(copyString(name, (int)strlen(name), &vm.strings)));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

static ObjModule *defineModule(const char *name) {
  ObjString *identifier = copyString(name, (int)strlen(name), &vm.strings);
  Value id = OBJ_VAL(identifier);
  push(id);
  ObjModule *module = newModule(identifier);
  Value mod = OBJ_VAL(module);
  push(mod);
  tableSet(&vm.globals, AS_STRING(id), mod);
  pop();
  pop();
  return module;
}

static void defineNativeModuleMethod(ObjModule *module, const char *name,
                                     NativeFn function) {
  ObjString *identifier = copyString(name, (int)strlen(name), &vm.strings);
  push(OBJ_VAL(module));
  Value id = OBJ_VAL(identifier);
  push(id);
  ObjNative *native = newNative(function);
  Value nativeFn = OBJ_VAL(native);
  push(nativeFn);
  tableSet(&module->fields, AS_STRING(id), nativeFn);
  pop();
  pop();
  pop();
}

static Value clockNative(int argCount, Value *args) {
  if (!checkArgCount(argCount, 0)) {
    return 0;
  };
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value exitNative(int argCount, Value *args) {
  if (argCount == 0) {
    exit(0);
  }
  if (!checkArgCount(argCount, 1)) {
    return 0;
  }
  if (!IS_NUMBER(args[0])) {
    runtimeError("Argument must be a number.");
    return 0;
  }
  exit(AS_NUMBER(args[0]));
}

static Value pushListNative(int argCount, Value *args) {
  if (!checkArgCount(argCount, 2)) {
    return 0;
  };
  if (!IS_LIST(args[0])) {
    runtimeError("Argument must be a list.");
  }
  ObjList *list = AS_LIST(args[0]);
  Value item = args[1];
  pushToList(list, item);
  return NIL_VAL;
}

static Value deleteListNative(int argCount, Value *args) {
  if (!checkArgCount(argCount, 2)) {
    return 0;
  }

  if (!IS_LIST(args[0])) {
    runtimeError("First argument must be a list.");
  }

  if (!IS_NUMBER(args[1])) {
    runtimeError("Argument must be number.");
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
  // defineNative("clock", clockNative);
  defineNative("exit", exitNative);

  defineNative("push", pushListNative);
  defineNative("remove", deleteListNative);
}

void registerListNatives() {
  static bool isRegistered = false;
  if (isRegistered)
    return;

  ObjModule *listModule = defineModule("List");
  defineNativeModuleMethod(listModule, "clock", clockNative);

  isRegistered = true;
}
