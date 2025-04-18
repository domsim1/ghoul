#ifndef ghoul_vm_h
#define ghoul_vm_h

#include "object.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
  ObjClosure *closure;
  uint8_t *ip;
  Value *slots;
} CallFrame;

typedef struct {
  ObjKlass *list;
  ObjKlass *map;
  ObjKlass *file;
  ObjKlass *string;
  ObjKlass *error;
  ObjKlass *pair;
} BuiltInKlass;

typedef struct {
  ObjString *init;
  ObjString *isError;
  ObjString *message;
  ObjString *r; 
  ObjString *g; 
  ObjString *b; 
  ObjString *a; 
  ObjString *x; 
  ObjString *y; 
  ObjString *z; 
} BuiltInStrings;

typedef struct {
  CallFrame frames[FRAMES_MAX];
  int frameCount;

  Value stack[STACK_MAX];
  Value *stackTop;
  Table globals;
  Table useStrings;
  Table strings;
  BuiltInKlass klass;
  BuiltInStrings string;
  ObjUpvalue *openUpvalues;
  bool shouldPanic;

  size_t bytesAllocated;
  size_t nextGC;
  Obj *objects;
  Obj *keep;
  int grayCount;
  int grayCapacity;
  Obj **grayStack;
} VM;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

// ObjString *createStracktrace();
void runtimeError(const char *format, ...);
void initVM();
void freeVM();
InterpretResult interpret(const char *source, const char *file);
void push(Value value);
Value pop();
Value peek(int distance);

#endif
