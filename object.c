#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObject(sizeof(type), objectType)

static Obj *allocateObject(size_t size, ObjType type) {
  Obj *object = (Obj *)reallocate(NULL, 0, size);
  object->type = type;
  object->isMarked = false;

  object->next = vm.objects;
  vm.objects = object;

#ifdef DEBUG_LOG_GC
  printf("%p allocate %zu for %d\n", (void *)object, size, type);
#endif

  return object;
}

ObjBoundMethod *newBoundMethod(Value receiver, ObjClosure *method) {
  ObjBoundMethod *bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);

  bound->receiver = receiver;
  bound->method = method;
  return bound;
}

ObjModule *newModule(ObjString *name) {
  ObjModule *module = ALLOCATE_OBJ(ObjModule, OBJ_MODULE);
  module->name = name;
  initTable(&module->methods);
  return module;
}

ObjClass *newClass(ObjString *name) {
  ObjClass *klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
  klass->name = name;
  initTable(&klass->methods);
  return klass;
}

ObjClosure *newClosure(ObjFunction *function) {
  ObjUpvalue **upvalues = ALLOCATE(ObjUpvalue *, function->upvalueCount);
  for (int i = 0; i < function->upvalueCount; i++) {
    upvalues[i] = NULL;
  }
  ObjClosure *closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
  closure->function = function;
  closure->upvalues = upvalues;
  closure->upvalueCount = function->upvalueCount;
  return closure;
}

ObjFunction *newFunction() {
  ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
  function->arity = 0;
  function->upvalueCount = 0;
  function->name = NULL;
  initChunk(&function->chunk);
  return function;
}

ObjInstance *newInstance(ObjClass *klass) {
  ObjInstance *instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
  instance->klass = klass;
  initTable(&instance->fields);
  return instance;
}

ObjNative *newNative(NativeFn function) {
  ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
  native->function = function;
  return native;
}

ObjList *newList() {
  ObjList *list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
  list->items = NULL;
  list->count = 0;
  list->capacity = 0;
  return list;
}

void pushToList(ObjList *list, Value value) {
  if (list->capacity < list->count + 1) {
    int oldCapacity = list->capacity;
    list->capacity = GROW_CAPACITY(oldCapacity);
    list->items = GROW_ARRAY(Value, list->items, oldCapacity, list->capacity);
  }
  list->items[list->count] = value;
  list->count++;
  return;
}

void storeToList(ObjList *list, int index, Value value) {
  list->items[index] = value;
}

Value indexFromList(ObjList *list, int index) { return list->items[index]; }

void deleteFromList(ObjList *list, int index) {
  for (int i = index; i < list->count - 1; i++) {
    list->items[i] = list->items[i + 1];
  }
  list->items[list->count - 1] = NIL_VAL;
  list->count--;
}

bool isValidListIndex(ObjList *list, int index) {
  if (index < 0 || index > list->count - 1) {
    return false;
  }
  return true;
}

ObjString *allocateString(char *chars, int length, uint32_t hash,
                          Table *stringTable) {
  ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length = length;
  string->hash = hash;
  string->chars = chars;

  push(OBJ_VAL(string));
  tableSet(stringTable, string, NIL_VAL);
  pop();
  return string;
}

uint32_t hashString(const char *key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

ObjString *takeString(char *chars, int length) {
  uint32_t hash = hashString(chars, length);
  return allocateString(chars, length, hash, &vm.strings);
}

ObjString *copyString(const char *chars, int length, Table *stringTable) {
  uint32_t hash = hashString(chars, length);
  ObjString *interned = tableFindString(stringTable, chars, length, hash);

  if (interned != NULL)
    return interned;

  char *heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocateString(heapChars, length, hash, stringTable);
}

ObjUpvalue *newUpvalue(Value *slot) {
  ObjUpvalue *upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
  upvalue->closed = NIL_VAL;
  upvalue->location = slot;
  upvalue->next = NULL;
  return upvalue;
}

static void printFunction(ObjFunction *function) {
  if (function->name == NULL) {
    printf("<script>");
    return;
  }
  printf("<fn %s>", function->name->chars);
}

static void printList(ObjList *list) {
  printf("[");
  for (int i = 0; i < list->count; i++) {
    printValue(list->items[i]);
    if (i < list->count - 1) {
      printf(", ");
    }
  }
  printf("]");
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_BOUND_METHOD:
    printFunction(AS_BOUND_METHOD(value)->method->function);
    break;
  case OBJ_MODULE:
    printf("<module %s>", AS_MODULE(value)->name->chars);
    break;
  case OBJ_CLASS:
    printf("<class %s>", AS_CLASS(value)->name->chars);
    break;
  case OBJ_CLOSURE:
    printFunction(AS_CLOSURE(value)->function);
    break;
  case OBJ_FUNCTION:
    printFunction(AS_FUNCTION(value));
    break;
  case OBJ_INSTANCE:
    printf("<instance %s>", AS_INSTANCE(value)->klass->name->chars);
    break;
  case OBJ_NATIVE:
    printf("<native fn %s>", AS_FUNCTION(value)->name->chars);
    break;
  case OBJ_LIST:
    printList(AS_LIST(value));
    break;
  case OBJ_STRING:
    printf("%s", AS_CSTRING(value));
    break;
  case OBJ_UPVALUE:
    printf("<upvalue>");
    break;
  }
}
