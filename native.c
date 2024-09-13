#include <string.h>
#include <time.h>

#include "object.h"
#include "value.h"
#include "vm.h"

static bool checkArgCount(int argCount, int expectedCount) {
  if (argCount != expectedCount) {
    runtimeError("Expected %d argugments but got %d.", expectedCount, argCount);
    return false;
  }
  return true;
}

static void defineNative(const char *name, NativeFn function) {
  push(OBJ_VAL(copyString(name, (int)strlen(name))));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

static void defineListNative(const char *name, NativeFn function) {
  push(OBJ_VAL(copyString(name, (int)strlen(name))));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.listMethods, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

static Value clockNative(int argCount, Value *args) {
  if (!checkArgCount(argCount, 0)) {
    return 0;
  };
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value pushListNative(int argCount, Value *args) {
  if (!checkArgCount(argCount - 1, 1)) {
    return 0;
  };
  ObjList *list = AS_LIST(args[0]);
  Value item = args[1];
  pushToList(list, item);
  return NIL_VAL;
}

static Value deleteListNative(int argCount, Value *args) {
  if (!checkArgCount(argCount - 1, 1)) {
    return 0;
  }

  if (!IS_NUMBER(args[1])) {
    runtimeError("Argugment must be number");
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

  defineListNative("push", pushListNative);
  defineListNative("remove", deleteListNative);
}
