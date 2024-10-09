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

ObjBoundNative *newBoundNative(Value receiver, ObjNative *native) {
  ObjBoundNative *bound = ALLOCATE_OBJ(ObjBoundNative, OBJ_BOUND_NATIVE);

  bound->receiver = receiver;
  bound->native = native;
  return bound;
}

ObjBoundMethod *newBoundMethod(Value receiver, ObjClosure *method) {
  ObjBoundMethod *bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);

  bound->receiver = receiver;
  bound->method = method;
  return bound;
}

ObjKlass *newKlass(ObjString *name, ObjType base) {
  ObjKlass *klass = ALLOCATE_OBJ(ObjKlass, OBJ_KLASS);
  klass->name = name;
  initTable(&klass->properties);
  klass->base = base;
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

ObjFunction *newFunction(const char *file) {
  ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
  function->arity = 0;
  function->upvalueCount = 0;
  function->name = NULL;
  function->variadic = false;
  initChunk(&function->chunk, file);
  return function;
}

ObjInstance *newInstance(ObjKlass *klass) {
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

ObjList *newList(ObjKlass *klass) {
  ObjList *list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
  list->items = NULL;
  list->klass = klass;
  list->count = 0;
  list->capacity = 0;
  initTable(&list->fields);
  return list;
}

ObjList *takeList(ObjKlass *klass, Value *values, int length) {
  ObjList *list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
  list->items = values;
  list->klass = klass;
  list->count = length;
  list->capacity = length + 1;
  initTable(&list->fields);
  return list;
}

ObjMap *newMap(ObjKlass *klass) {
  ObjMap *map = ALLOCATE_OBJ(ObjMap, OBJ_MAP);
  map->klass = klass;
  initTable(&map->items);
  initTable(&map->fields);
  return map;
}

ObjFile *newFile(ObjKlass *klass) {
  ObjFile *file = ALLOCATE_OBJ(ObjFile, OBJ_FILE);
  file->file = NULL;
  file->klass = klass;
  initTable(&file->fields);
  return file;
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

void deleteFromList(ObjList *list, int start, int end) {
  int range = end - start + 1;

  for (int i = start; i < list->count - range; i++) {
    list->items[i] = list->items[i + range];
  }

  for (int i = list->count - range; i < list->count; i++) {
    list->items[i] = NIL_VAL;
  }

  list->count -= range;
}

bool isValidListRange(ObjList *list, int start, int end) {
  if (start < 0 || end >= list->count || start > end) {
    return false;
  }
  return true;
}

bool isValidListIndex(ObjList *list, int index) {
  if (index < 0 || index > list->count - 1) {
    return false;
  }
  return true;
}

ObjString *allocateString(char *chars, int length, uint32_t hash,
                          Table *stringTable, ObjKlass *klass) {
  ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length = length;
  string->hash = hash;
  string->chars = chars;
  string->klass = klass;
  initTable(&string->fields);

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
  return allocateString(chars, length, hash, &vm.strings, vm.klass.string);
}

ObjString *copyString(const char *chars, int length, Table *stringTable) {
  uint32_t hash = hashString(chars, length);
  ObjString *interned = tableFindString(stringTable, chars, length, hash);

  if (interned != NULL)
    return interned;

  char *heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocateString(heapChars, length, hash, stringTable, vm.klass.string);
}

ObjString *copyEscString(const char *chars, int length, Table *stringTable,
                         ObjKlass *klass) {
  char escString[length];
  int escLen = 0;
  bool escMode = false;
  for (int i = 0; i < length; i++) {
    char c = chars[i];
    if (escMode) {
      switch (c) {
      case '"':
      case '\\':
        break;
      case 'n':
        c = '\n';
        break;
      case 'r':
        c = '\r';
        break;
      }
      escMode = false;
    } else if (c == '\\') {
      escMode = true;
      continue;
    }
    escString[escLen++] = c;
  }
  uint32_t hash = hashString(escString, escLen);
  ObjString *interned = tableFindString(stringTable, escString, escLen, hash);

  if (interned != NULL)
    return interned;

  char *heapChars = ALLOCATE(char, escLen + 1);
  memcpy(heapChars, escString, escLen);
  heapChars[escLen] = '\0';
  return allocateString(heapChars, escLen, hash, stringTable, klass);
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
  printf("<fn '%s'>", function->name->chars);
}

static void printList(ObjList *list) {
  printf("[");
  for (int i = 0; i < list->count; i++) {
    Value value = list->items[i];
    if (IS_STRING(value)) {
      printf("'");
      printValue(value);
      printf("'");
    } else {
      printValue(value);
    }
    if (i < list->count - 1) {
      printf(", ");
    }
  }
  printf("]");
}

static void printMap(ObjMap *map) {
  printf("{");
  for (int i = 0; i < map->items.capacity; i++) {
    Entry entry = map->items.entries[i];
    if (entry.key == NULL) {
      continue;
    }
    printf("\"%s\":", entry.key->chars);
    printValue(entry.value);
    printf(",");
  }
  printf("}");
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_BOUND_METHOD:
    printFunction(AS_BOUND_METHOD(value)->method->function);
    break;
  case OBJ_BOUND_NATIVE:
    printf("<bound native fn>");
    break;
  case OBJ_KLASS:
    printf("<class '%s'>", AS_KLASS(value)->name->chars);
    break;
  case OBJ_CLOSURE:
    printFunction(AS_CLOSURE(value)->function);
    break;
  case OBJ_FUNCTION:
    printFunction(AS_FUNCTION(value));
    break;
  case OBJ_INSTANCE:
    printf("<instance '%s'>", AS_INSTANCE(value)->klass->name->chars);
    break;
  case OBJ_NATIVE:
    printf("<native fn>");
    break;
  case OBJ_LIST:
    printList(AS_LIST(value));
    break;
  case OBJ_MAP:
    printMap(AS_MAP(value));
    break;
  case OBJ_STRING:
    printf("%s", AS_CSTRING(value));
    break;
  case OBJ_UPVALUE:
    printf("<upvalue>");
    break;
  case OBJ_FILE:
    printf("<instance '%s'>", AS_FILE(value)->klass->name->chars);
    break;
  }
}
