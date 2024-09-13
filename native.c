#include <time.h>

#include "object.h"
#include "value.h"
#include "vm.h"

static void defineNative(const char *name, NativeFn function) {
  push(OBJ_VAL(copyString(name, (int)strlen(name))));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

static Value clockNative(int argCount, Value *args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value pushListNative(int argCount, Value *args) {
  if (argCount != 2 || !IS_LIST(args[0])) {
    return NIL_VAL;
  }
  ObjList *list = AS_LIST(args[0]);
  Value item = args[1];
  pushToList(list, item);
  return NIL_VAL;
}

static void defineListNative(const char *name, NativeFn function) {
  push(OBJ_VAL(copyString(name, (int)strlen(name))));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.listMethods, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

static Value deleteListNative(int argCount, Value *args) {
  if (argCount != 2 || !IS_LIST(args[0]) || !IS_NUMBER(args[1])) {
    return NIL_VAL;
  }

  ObjList *list = AS_LIST(args[0]);
  int index = AS_NUMBER(args[1]);

  if (!isValidListIndex(list, index)) {
    return NIL_VAL;
  }

  deleteFromList(list, index);
  return NIL_VAL;
}

void registerNatives() {
  defineNative("clock", clockNative);

  defineListNative("push", pushListNative);
  defineListNative("remove", deleteListNative);
}
