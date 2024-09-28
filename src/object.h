#ifndef ghoul_object_h
#define ghoul_object_h
#include <stdio.h>

#include "chunk.h"
#include "table.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_BOUND_NATIVE(value) isObjType(value, OBJ_BOUND_NATIVE)
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_LIST(value) isObjType(value, OBJ_LIST)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_FILE(value) isObjType(value, OBJ_FILE)

#define AS_BOUND_NATIVE(value) ((ObjBoundNative *)AS_OBJ(value))
#define AS_BOUND_METHOD(value) ((ObjBoundMethod *)AS_OBJ(value))
#define AS_KLASS(value) ((ObjKlass *)AS_OBJ(value))
#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_LIST(value) ((ObjList *)AS_OBJ(value))
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)
#define AS_FILE(value) ((ObjFile *)AS_OBJ(value))

typedef enum {
  OBJ_BOUND_METHOD,
  OBJ_BOUND_NATIVE,
  OBJ_CLASS,
  OBJ_CLOSURE,
  OBJ_FUNCTION,
  OBJ_INSTANCE,
  OBJ_NATIVE,
  OBJ_LIST,
  OBJ_STRING,
  OBJ_UPVALUE,
  OBJ_FILE,
} ObjType;

struct Obj {
  ObjType type;
  bool isMarked;
  struct Obj *next;
};

typedef struct {
  Obj obj;
  int arity;
  int upvalueCount;
  Chunk chunk;
  ObjString *name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value *args);

typedef struct {
  Obj obj;
  NativeFn function;
} ObjNative;

struct ObjString {
  Obj obj;
  int length;
  uint32_t hash;
  char *chars;
};

typedef struct ObjUpvalue {
  Obj obj;
  Value *location;
  Value closed;
  struct ObjUpvalue *next;
} ObjUpvalue;

typedef struct {
  Obj obj;
  ObjFunction *function;
  ObjUpvalue **upvalues;
  int upvalueCount;
} ObjClosure;

typedef struct {
  Obj obj;
  ObjString *name;
  ObjType base;
  Table methods;
} ObjKlass;

typedef struct {
  Obj obj;
  ObjKlass *klass;
  Table fields;
} ObjInstance;

typedef struct {
  Obj obj;
  Value receiver;
  ObjClosure *method;
} ObjBoundMethod;

typedef struct {
  Obj obj;
  Value receiver;
  ObjNative *native;
} ObjBoundNative;

typedef struct {
  Obj obj;
  ObjKlass *klass;
  int count;
  int capacity;
  Value *items;
  Table fields;
} ObjList;

typedef struct {
  Obj obj;
  FILE *file;
  ObjKlass *klass;
  Table fields;
} ObjFile;

ObjBoundMethod *newBoundMethod(Value receiver, ObjClosure *method);
ObjBoundNative *newBoundNative(Value receiver, ObjNative *native);
ObjKlass *newClass(ObjString *name, ObjType base);
ObjClosure *newClosure(ObjFunction *function);
ObjFunction *newFunction();
ObjInstance *newInstance(ObjKlass *klass);
ObjNative *newNative(NativeFn function);
ObjList *newList(ObjKlass *klass);
ObjString *takeString(char *chars, int length);
ObjString *copyString(const char *chars, int length, Table *stringTable);
ObjString *copyEscString(const char *chars, int length, Table *stringTable);
ObjUpvalue *newUpvalue(Value *slot);
ObjFile *newFile(ObjKlass *klass);
void printObject(Value value);

void pushToList(ObjList *list, Value value);
void storeToList(ObjList *list, int index, Value value);
Value indexFromList(ObjList *list, int index);
void deleteFromList(ObjList *list, int start, int end);
bool isValidListRange(ObjList *list, int start, int end);
bool isValidListIndex(ObjList *list, int index);
uint32_t hashString(const char *key, int length);
ObjString *allocateString(char *chars, int length, uint32_t hash,
                          Table *stringTable);

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
