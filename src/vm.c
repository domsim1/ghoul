#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "chunk.h"
#include "compiler.h"
#include "memory.h"
#include "native/native.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

VM vm;

static void resetStack() {
  vm.stackTop = vm.stack;
  vm.frameCount = 0;
  vm.openUpvalues = NULL;
}

// ObjString *createStracktrace() {
//   int len = 0;
//   char *stacktrace = ALLOCATE(char, 0);
//   for (int i = vm.frameCount - 1; i >= 0; i--) {
//     CallFrame *frame = &vm.frames[i];
//     ObjFunction *function = frame->closure->function;
//     size_t instruction = frame->ip - function->chunk.code - 1;
//     int strLen =
//         snprintf(NULL, 0, "[line %d of %s] in %s%s\n",
//                  getLine(&function->chunk, instruction),
//                  getLineFileName(&function->chunk, instruction),
//                  function->name == NULL ? "script" : function->name->chars,
//                  function->name == NULL ? "" : "()");
//     char str[strLen + 1];
//     int oldlen = len;
//     len += strLen + 1;
//     snprintf(str, strLen + 1, "[line %d of %s] in %s%s\n",
//              getLine(&function->chunk, instruction),
//              getLineFileName(&function->chunk, instruction),
//              function->name == NULL ? "script" : function->name->chars,
//              function->name == NULL ? "" : "()");
//     stacktrace = GROW_ARRAY(char, stacktrace, oldlen, len + 1);
//     strcat(stacktrace, str);
//   }
//   ObjString *stack = copyString(stacktrace, len - 1, &vm.strings);
//   FREE(char, stacktrace);
//   return stack;
// }

void runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  for (int i = vm.frameCount - 1; i >= 0; i--) {
    CallFrame *frame = &vm.frames[i];
    ObjFunction *function = frame->closure->function;
    size_t instruction = frame->ip - function->chunk.code - 1;
    fprintf(stderr, "[line %d of %s] in ",
            getLine(&function->chunk, instruction),
            getFileName(&function->chunk));
    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->chars);
    }
  }

  resetStack();
}

void initVM() {
  resetStack();
  vm.objects = NULL;
  vm.bytesAllocated = 0;
  vm.nextGC = 1024 * 1024;

  vm.grayCount = 0;
  vm.grayCapacity = 0;
  vm.grayStack = NULL;

  initTable(&vm.globals);
  initTable(&vm.strings);
  initTable(&vm.useStrings);

  vm.string.init = NULL;
  vm.string.isError = NULL;
  vm.string.message = NULL;
  vm.string.r = NULL;
  vm.string.g = NULL;
  vm.string.b = NULL;
  vm.string.a = NULL;
  vm.string.x = NULL;
  vm.string.y = NULL;
  vm.string.z = NULL;
  vm.string.init = copyString("init", 4, &vm.strings);
  vm.string.isError = copyString("_isError_", 9, &vm.strings);
  vm.string.message = copyString("message", 7, &vm.strings);
  vm.string.r = copyString("r", 1, &vm.strings);
  vm.string.g = copyString("g", 1, &vm.strings);
  vm.string.b = copyString("b", 1, &vm.strings);
  vm.string.a = copyString("a", 1, &vm.strings);
  vm.string.x = copyString("x", 1, &vm.strings);
  vm.string.y = copyString("y", 1, &vm.strings);
  vm.string.z = copyString("z", 1, &vm.strings);

  vm.keep = NULL;
  vm.shouldPanic = false;

  registerBuiltInKlasses();
  registerNatives();
}

void freeVM() {
  freeTable(&vm.strings);
  freeTable(&vm.globals);
  freeTable(&vm.useStrings);
  vm.string.init = NULL;
  vm.string.isError = NULL;
  vm.string.message = NULL;
  vm.string.r = NULL;
  vm.string.g = NULL;
  vm.string.b = NULL;
  vm.string.a = NULL;
  vm.string.x = NULL;
  vm.string.y = NULL;
  vm.string.z = NULL;
  vm.klass.list = NULL;
  vm.klass.file = NULL;
  vm.klass.string = NULL;
  vm.klass.error = NULL;
  vm.klass.pair = NULL;
  vm.klass.map = NULL;
  vm.keep = NULL;
  freeObjects();
}

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

Value peek(int distance) { return vm.stackTop[-1 - distance]; }

static ObjUpvalue *captureUpvalue(Value *local);

static bool call(ObjClosure *closure, int argCount) {
  if (!closure->function->variadic) {
    if (argCount != closure->function->arity) {
      runtimeError("Expected %d argugments but got %d.",
                   closure->function->arity, argCount);
      return false;
    }
  } else {
    if (argCount < closure->function->arity - 1) {
      runtimeError("Expected at least %d argugments but got %d.",
                   closure->function->arity - 1, argCount);
      return false;
    }
    ObjList *variadicArgs = newList(vm.klass.list);
    vm.keep = (Obj *)variadicArgs;
    int i = argCount - closure->function->arity;
    int popback = i + 1;
    while (argCount > closure->function->arity - 1) {
      argCount--;
      pushToList(variadicArgs, peek(i--));
    }
    vm.stackTop -= popback;
    push(OBJ_VAL(variadicArgs));
    vm.keep = NULL;
    argCount++;
  }

  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  CallFrame *frame = &vm.frames[vm.frameCount++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;
  frame->slots = vm.stackTop - argCount - 1;
  return true;
}

static bool callNative(NativeFn native, int argCount) {
  Value result = native(argCount + 1, vm.stackTop - (argCount + 1));
  if (vm.shouldPanic) {
    vm.shouldPanic = false;
    return false;
  }
  vm.stackTop -= argCount + 1;
  push(result);
  return true;
}

static bool initClass(ObjKlass *klass, Value initializer, int argCount) {
  switch (klass->base) {
  case OBJ_LIST:
    vm.stackTop[-argCount - 1] = OBJ_VAL(newList(klass));
    break;
  case OBJ_MAP:
    vm.stackTop[-argCount - 1] = OBJ_VAL(newMap(klass));
    break;
  case OBJ_FILE:
    vm.stackTop[-argCount - 1] = OBJ_VAL(newFile(klass));
    break;
  case OBJ_STRING:
    vm.stackTop[-argCount - 1] =
        OBJ_VAL(copyEscString("", 0, &vm.strings, klass));
    break;
  default:
    vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));
  }
  return call(AS_CLOSURE(initializer), argCount);
}

static bool callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
    case OBJ_BOUND_METHOD: {
      ObjBoundMethod *bound = AS_BOUND_METHOD(callee);
      vm.stackTop[-argCount - 1] = bound->receiver;
      return call(bound->method, argCount);
    }
    case OBJ_BOUND_NATIVE: {
      ObjBoundNative *bound = AS_BOUND_NATIVE(callee);
      vm.stackTop[-argCount - 1] = bound->receiver;
      NativeFn native = bound->native->function;
      return callNative(native, argCount);
    }
    case OBJ_KLASS: {
      ObjKlass *klass = AS_KLASS(callee);
      Value initializer;
      if (tableGet(&klass->properties, vm.string.init, &initializer)) {
        if (IS_NATIVE(initializer)) {
          return callNative(AS_NATIVE(initializer), argCount);
        }
        return initClass(klass, initializer, argCount);
      } else if (argCount != 0) {
        runtimeError("Expected 0 argugments but got %d.", argCount);
        return false;
      }
      pop();
      push(OBJ_VAL(newInstance(klass)));
      return true;
    }
    case OBJ_CLOSURE:
      return call(AS_CLOSURE(callee), argCount);
    case OBJ_NATIVE: {
      return callNative(AS_NATIVE(callee), argCount);
    }
    default:
      break;
    }
  }
  runtimeError("Can only call function and classes.");
  return false;
}

static bool invokeFromClass(ObjKlass *klass, ObjString *name, int argCount) {
  Value method;
  if (!tableGet(&klass->properties, name, &method)) {
    runtimeError("Undefined property '%s'.", name->chars);
    return false;
  }
  if (IS_NATIVE(method)) {
    return callNative(AS_NATIVE(method), argCount);
  }
  return call(AS_CLOSURE(method), argCount);
}

static bool invoke(ObjString *name, int argCount) {
  Value receiver = peek(argCount);
  ObjKlass *klass = NULL;
  Table *fields = NULL;
  if (IS_INSTANCE(receiver)) {
    ObjInstance *instance = AS_INSTANCE(receiver);
    klass = instance->klass;
    fields = &instance->fields;
  } else if (IS_LIST(receiver)) {
    ObjList *list = AS_LIST(receiver);
    klass = list->klass;
    fields = &list->fields;
  } else if (IS_MAP(receiver)) {
    ObjMap *map = AS_MAP(receiver);
    klass = map->klass;
    fields = &map->fields;
  } else if (IS_FILE(receiver)) {
    ObjFile *file = AS_FILE(receiver);
    klass = file->klass;
    fields = &file->fields;
  } else if (IS_STRING(receiver)) {
    ObjString *string = AS_STRING(receiver);
    klass = string->klass;
    fields = &string->fields;
  } else {
    runtimeError("Only instances have methods.");
    return false;
  }
  Value value;
  if (tableGet(fields, name, &value)) {
    vm.stackTop[-argCount - 1] = value;
    return callValue(value, argCount);
  }
  return invokeFromClass(klass, name, argCount);
}

static bool bindKlassProp(ObjKlass *klass, ObjString *name, bool ignoreError) {
  Value prop;
  if (!tableGet(&klass->properties, name, &prop)) {
    if (!ignoreError) {
      runtimeError("Undefined property '%s'.", name->chars);
    }
    return false;
  }

  if (IS_NATIVE(prop)) {
    ObjBoundNative *bound = newBoundNative(peek(0), (ObjNative *)AS_OBJ(prop));
    pop();
    push(OBJ_VAL(bound));
    return true;
  } else if (IS_CLOSURE(prop)) {
    ObjBoundMethod *bound = newBoundMethod(peek(0), AS_CLOSURE(prop));
    pop();
    push(OBJ_VAL(bound));
    return true;
  }

  pop();
  push(prop);
  return true;
}

static ObjUpvalue *captureUpvalue(Value *local) {
  ObjUpvalue *prevUpvalue = NULL;
  ObjUpvalue *upvalue = vm.openUpvalues;
  while (upvalue != NULL && upvalue->location > local) {
    prevUpvalue = upvalue;
    upvalue = upvalue->next;
  }

  if (upvalue != NULL && upvalue->location == local) {
    return upvalue;
  }

  ObjUpvalue *createdUpvalue = newUpvalue(local);
  createdUpvalue->next = upvalue;

  if (prevUpvalue == NULL) {
    vm.openUpvalues = createdUpvalue;
  } else {
    prevUpvalue->next = createdUpvalue;
  }

  return createdUpvalue;
}

static void closeUpvalues(Value *last) {
  while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last) {
    ObjUpvalue *upvalue = vm.openUpvalues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    vm.openUpvalues = upvalue->next;
  }
}

static void defineMethod(ObjString *name) {
  Value method = peek(0);
  ObjKlass *klass = AS_KLASS(peek(1));
  tableSet(&klass->properties, name, method);
  pop();
  return;
}

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenateStrings() {
  ObjString *b = AS_STRING(peek(0));
  ObjString *a = AS_STRING(peek(1));

  int length = a->length + b->length;
  char *chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString *result = takeString(chars, length);
  pop();
  pop();
  push(OBJ_VAL(result));
}

static void concatenateLists() {
  ObjList *b = AS_LIST(peek(0));
  ObjList *a = AS_LIST(peek(1));

  int length = a->count + b->count;
  Value *values = ALLOCATE(Value, length + 1);
  memcpy(values, a->items, sizeof(Value) * a->count);
  memcpy(values + a->count, b->items, sizeof(Value) * b->count);

  ObjList *result = takeList(a->klass, values, length);
  pop();
  pop();
  push(OBJ_VAL(result));
}

static InterpretResult run() {
  CallFrame *frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*(frame->ip++))
#define READ_SHORT()                                                           \
  (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT()                                                        \
  (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_CONSTANT_SHORT()                                                  \
  (frame->closure->function->chunk.constants.values[READ_SHORT()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_STRING_SHORT() AS_STRING(READ_CONSTANT_SHORT())
#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                          \
      runtimeError("Operands must be numbers.");                               \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    double b = AS_NUMBER(pop());                                               \
    double a = AS_NUMBER(pop());                                               \
    push(valueType(a op b));                                                   \
  } while (false)
#define BITWISE_OP(valueType, op)                                              \
  do {                                                                         \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                          \
      runtimeError("Operands must be numbers.");                               \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    int b = (int)AS_NUMBER(pop());                                             \
    int a = (int)AS_NUMBER(pop());                                             \
    push(valueType((double)(a op b)));                                         \
  } while (false)
#define MATH_OP(valueType, op)                                                 \
  do {                                                                         \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                          \
      runtimeError("Operands must be numbers.");                               \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    double b = AS_NUMBER(pop());                                               \
    double a = AS_NUMBER(pop());                                               \
    push(valueType(op(a, b)));                                                 \
  } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(
        &frame->closure->function->chunk,
        (int)(frame->ip - frame->closure->function->chunk.code));
#endif

    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
    case OP_CONSTANT: {
      Value constant = READ_CONSTANT();
      push(constant);
      break;
    }
    case OP_CONSTANT_SHORT: {
      Value constant = READ_CONSTANT_SHORT();
      push(constant);
      break;
    }
    case OP_NIL:
      push(NIL_VAL);
      break;
    case OP_TRUE:
      push(BOOL_VAL(true));
      break;
    case OP_FALSE:
      push(BOOL_VAL(false));
      break;
    case OP_POP:
      pop();
      break;
    case OP_GET_LOCAL: {
      uint8_t slot = READ_BYTE();
      push(frame->slots[slot]);
      break;
    }
    case OP_GET_LOCAL_SHORT: {
      uint16_t slot = READ_SHORT();
      push(frame->slots[slot]);
      break;
    }
    case OP_GET_GLOBAL: {
      ObjString *name = READ_STRING();
      Value value;
      if (!tableGet(&vm.globals, name, &value)) {
        runtimeError("Undefined '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      push(value);
      break;
    }
    case OP_GET_GLOBAL_SHORT: {
      ObjString *name = READ_STRING_SHORT();
      Value value;
      if (!tableGet(&vm.globals, name, &value)) {
        runtimeError("Undefined '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      push(value);
      break;
    }
    case OP_SET_GLOBAL: {
      ObjString *name = READ_STRING();
      if (tableSet(&vm.globals, name, peek(0))) {
        tableDelete(&vm.globals, name);
        runtimeError("Undefined '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_SET_GLOBAL_SHORT: {
      ObjString *name = READ_STRING_SHORT();
      if (tableSet(&vm.globals, name, peek(0))) {
        tableDelete(&vm.globals, name);
        runtimeError("Undefined '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_SET_LOCAL: {
      uint8_t slot = READ_BYTE();
      frame->slots[slot] = peek(0);
      break;
    }
    case OP_SET_LOCAL_SHORT: {
      uint16_t slot = READ_SHORT();
      frame->slots[slot] = peek(0);
      break;
    }
    case OP_DEFINE_GLOBAL: {
      ObjString *name = READ_STRING();
      if (!tableSet(&vm.globals, name, peek(0))) {
        runtimeError("Global %s is already defined.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      pop();
      break;
    }
    case OP_DEFINE_GLOBAL_SHORT: {
      ObjString *name = READ_STRING_SHORT();
      if (!tableSet(&vm.globals, name, peek(0))) {
        runtimeError("Global %s is already defined.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      pop();
      break;
    }
    case OP_NEGATE: {
      if (!IS_NUMBER(peek(0))) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      push(NUMBER_VAL(-AS_NUMBER(pop())));
      break;
    }
    case OP_BITWISE_NOT: {
      if (!IS_NUMBER(peek(0))) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      push(NUMBER_VAL((double)~(int)AS_NUMBER(pop())));
      break;
    }
    case OP_GET_UPVALUE: {
      uint8_t slot = READ_BYTE();
      push(*frame->closure->upvalues[slot]->location);
      break;
    }
    case OP_GET_UPVALUE_SHORT: {
      uint8_t slot = READ_SHORT();
      push(*frame->closure->upvalues[slot]->location);
      break;
    }
    case OP_SET_UPVALUE: {
      uint8_t slot = READ_BYTE();
      *frame->closure->upvalues[slot]->location = peek(0);
      break;
    }
    case OP_SET_UPVALUE_SHORT: {
      uint8_t slot = READ_SHORT();
      *frame->closure->upvalues[slot]->location = peek(0);
      break;
    }
    case OP_GET_PROPERTY: {
      ObjKlass *klass = NULL;
      Table *fields = NULL;
      if (IS_INSTANCE(peek(0))) {
        ObjInstance *instance = AS_INSTANCE(peek(0));
        klass = instance->klass;
        fields = &instance->fields;
      } else if (IS_LIST(peek(0))) {
        ObjList *list = AS_LIST(peek(0));
        klass = list->klass;
        fields = &list->fields;
      } else if (IS_MAP(peek(0))) {
        ObjMap *map = AS_MAP(peek(0));
        klass = map->klass;
        fields = &map->fields;
      } else if (IS_FILE(peek(0))) {
        ObjFile *file = AS_FILE(peek(0));
        klass = file->klass;
        fields = &file->fields;
      } else if (IS_STRING(peek(0))) {
        ObjString *string = AS_STRING(peek(0));
        klass = string->klass;
        fields = &string->fields;
      } else {
        runtimeError("Only instances have properties.");
        return INTERPRET_RUNTIME_ERROR;
      }
      ObjString *name = READ_STRING();
      Value value;
      if (tableGet(fields, name, &value)) {
        pop();
        push(value);
        break;
      }
      if (!bindKlassProp(klass, name, false)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_GET_PROPERTY_SHORT: {
      ObjKlass *klass = NULL;
      Table *fields = NULL;
      if (IS_INSTANCE(peek(0))) {
        ObjInstance *instance = AS_INSTANCE(peek(0));
        klass = instance->klass;
        fields = &instance->fields;
      } else if (IS_LIST(peek(0))) {
        ObjList *list = AS_LIST(peek(0));
        klass = list->klass;
        fields = &list->fields;
      } else if (IS_MAP(peek(0))) {
        ObjMap *map = AS_MAP(peek(0));
        klass = map->klass;
        fields = &map->fields;
      } else if (IS_FILE(peek(0))) {
        ObjFile *file = AS_FILE(peek(0));
        klass = file->klass;
        fields = &file->fields;
      } else if (IS_STRING(peek(0))) {
        ObjString *string = AS_STRING(peek(0));
        klass = string->klass;
        fields = &string->fields;
      } else {
        runtimeError("Only instances have properties.");
        return INTERPRET_RUNTIME_ERROR;
      }
      ObjString *name = READ_STRING_SHORT();
      Value value;
      if (tableGet(fields, name, &value)) {
        pop();
        push(value);
        break;
      }
      if (!bindKlassProp(klass, name, false)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_SET_PROPERTY: {
      Table *fields = NULL;
      if (IS_INSTANCE(peek(1))) {
        ObjInstance *instance = AS_INSTANCE(peek(1));
        fields = &instance->fields;
      } else if (IS_LIST(peek(1))) {
        ObjList *list = AS_LIST(peek(1));
        fields = &list->fields;
      } else if (IS_MAP(peek(0))) {
        ObjMap *map = AS_MAP(peek(0));
        fields = &map->fields;
      } else if (IS_STRING(peek(0))) {
        ObjString *string = AS_STRING(peek(0));
        fields = &string->fields;
      } else if (IS_FILE(peek(0))) {
        ObjFile *file = AS_FILE(peek(0));
        fields = &file->fields;
      } else {
        runtimeError("Only instances have fields.");
        return INTERPRET_RUNTIME_ERROR;
      }
      ObjString *name = READ_STRING();
      tableSet(fields, name, peek(0));
      Value value = pop();
      pop();
      push(value);
      break;
    }
    case OP_SET_PROPERTY_SHORT: {
      Table *fields = NULL;
      if (IS_INSTANCE(peek(1))) {
        ObjInstance *instance = AS_INSTANCE(peek(1));
        fields = &instance->fields;
      } else if (IS_LIST(peek(1))) {
        ObjList *list = AS_LIST(peek(1));
        fields = &list->fields;
      } else if (IS_MAP(peek(0))) {
        ObjMap *map = AS_MAP(peek(0));
        fields = &map->fields;
      } else if (IS_STRING(peek(0))) {
        ObjString *string = AS_STRING(peek(0));
        fields = &string->fields;
      } else if (IS_FILE(peek(0))) {
        ObjFile *file = AS_FILE(peek(0));
        fields = &file->fields;
      } else {
        runtimeError("Only instances have fields.");
        return INTERPRET_RUNTIME_ERROR;
      }
      ObjString *name = READ_STRING_SHORT();
      tableSet(fields, name, peek(0));
      Value value = pop();
      pop();
      push(value);
      break;
    }
    case OP_GET_SUPER: {
      ObjString *name = READ_STRING();
      ObjKlass *superclass = AS_KLASS(pop());

      if (!bindKlassProp(superclass, name, false)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_GET_SUPER_SHORT: {
      ObjString *name = READ_STRING_SHORT();
      ObjKlass *superclass = AS_KLASS(pop());

      if (!bindKlassProp(superclass, name, false)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_EQUAL: {
      Value a = pop();
      Value b = pop();
      push(BOOL_VAL(valuesEqual(a, b)));
      break;
    }
    case OP_GREATER:
      BINARY_OP(BOOL_VAL, >);
      break;
    case OP_LESS:
      BINARY_OP(BOOL_VAL, <);
      break;
    case OP_BANG_EQUAL: {
      Value a = pop();
      Value b = pop();
      push(BOOL_VAL(!valuesEqual(a, b)));
      break;
    }
    case OP_GREATER_EQUAL: {
      BINARY_OP(BOOL_VAL, >=);
      break;
    }
    case OP_LESS_EQUAL: {
      BINARY_OP(BOOL_VAL, <=);
      break;
    }
    case OP_ADD:
      BINARY_OP(NUMBER_VAL, +);
      break;
    case OP_CONCAT:
      if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
        concatenateStrings();
      } else if (IS_LIST(peek(0)) && IS_LIST(peek(1))) {
        concatenateLists();
      } else {
        runtimeError("Can only concat two strings or lists.");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    case OP_SUBTRACT:
      BINARY_OP(NUMBER_VAL, -);
      break;
    case OP_MULTIPLY:
      BINARY_OP(NUMBER_VAL, *);
      break;
    case OP_DIVIDE:
      BINARY_OP(NUMBER_VAL, /);
      break;
    case OP_MOD:
      MATH_OP(NUMBER_VAL, fmod);
      break;
    case OP_EXPONENTIATION:
      MATH_OP(NUMBER_VAL, pow);
      break;
    case OP_BITWISE_AND:
      BITWISE_OP(NUMBER_VAL, &);
      break;
    case OP_BITWISE_OR:
      BITWISE_OP(NUMBER_VAL, |);
      break;
    case OP_BITWISE_XOR:
      BITWISE_OP(NUMBER_VAL, ^);
      break;
    case OP_BITWISE_LEFT_SHIFT:
      BITWISE_OP(NUMBER_VAL, <<);
      break;
    case OP_BITWISE_RIGHT_SHIFT:
      BITWISE_OP(NUMBER_VAL, >>);
      break;
    case OP_NOT:
      push(BOOL_VAL(isFalsey(pop())));
      break;
    case OP_PRINT: {
      printValue(pop());
      printf("\n");
      break;
    }
    case OP_JUMP: {
      uint16_t offset = READ_SHORT();
      frame->ip += offset;
      break;
    }
    case OP_JUMP_IF_FALSE: {
      uint16_t offset = READ_SHORT();
      if (isFalsey(peek(0)))
        frame->ip += offset;
      break;
    }
    case OP_LOOP: {
      uint16_t offset = READ_SHORT();
      frame->ip -= offset;
      break;
    }
    case OP_CALL: {
      int argCount = READ_BYTE();
      if (!callValue(peek(argCount), argCount)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_CALL_SHORT: {
      int argCount = READ_SHORT();
      if (!callValue(peek(argCount), argCount)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_INVOKE: {
      ObjString *method = READ_STRING();
      int argCount = READ_BYTE();
      if (!invoke(method, argCount)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_INVOKE_SHORT: {
      ObjString *method = READ_STRING_SHORT();
      int argCount = READ_SHORT();
      if (!invoke(method, argCount)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_SUPER_INVOKE: {
      ObjString *method = READ_STRING();
      int argCount = READ_BYTE();
      ObjKlass *superclass = AS_KLASS(pop());
      if (!invokeFromClass(superclass, method, argCount)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_SUPER_INVOKE_SHORT: {
      ObjString *method = READ_STRING_SHORT();
      int argCount = READ_SHORT();
      ObjKlass *superclass = AS_KLASS(pop());
      if (!invokeFromClass(superclass, method, argCount)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_CLOSURE: {
      ObjFunction *function = AS_FUNCTION(READ_CONSTANT());
      ObjClosure *closure = newClosure(function);
      push(OBJ_VAL(closure));
      for (int i = 0; i < closure->upvalueCount; i++) {
        uint8_t isLocal = READ_BYTE();
        uint16_t index = READ_SHORT();
        if (isLocal) {
          closure->upvalues[i] = captureUpvalue(frame->slots + index);
        } else {
          closure->upvalues[i] = frame->closure->upvalues[index];
        }
      }
      break;
    }
    case OP_CLOSURE_SHORT: {
      ObjFunction *function = AS_FUNCTION(READ_CONSTANT_SHORT());
      ObjClosure *closure = newClosure(function);
      push(OBJ_VAL(closure));
      for (int i = 0; i < closure->upvalueCount; i++) {
        uint8_t isLocal = READ_BYTE();
        uint16_t index = READ_SHORT();
        if (isLocal) {
          closure->upvalues[i] = captureUpvalue(frame->slots + index);
        } else {
          closure->upvalues[i] = frame->closure->upvalues[index];
        }
      }
      break;
    }
    case OP_CLOSE_UPVALUE:
      closeUpvalues(vm.stackTop - 1);
      pop();
      break;
    case OP_RETURN: {
      Value result = pop();
      closeUpvalues(frame->slots);
      vm.frameCount--;
      if (vm.frameCount == 0) {
        pop();
        return INTERPRET_OK;
      }

      vm.stackTop = frame->slots;
      push(result);
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_CLASS:
      push(OBJ_VAL(newKlass(READ_STRING(), OBJ_KLASS)));
      break;
    case OP_CLASS_SHORT:
      push(OBJ_VAL(newKlass(READ_STRING_SHORT(), OBJ_KLASS)));
      break;
    case OP_INHERIT: {
      Value superclass = peek(1);
      if (!IS_KLASS(superclass)) {
        runtimeError("Superclass must be a class.");
        return INTERPRET_RUNTIME_ERROR;
      }
      ObjKlass *subclass = AS_KLASS(peek(0));
      ObjKlass *super = AS_KLASS(superclass);
      tableAddAll(&super->properties, &subclass->properties);
      subclass->base = super->base;
      pop();
      break;
    }
    case OP_PROPERTY:
      defineMethod(READ_STRING());
      break;
    case OP_PROPERTY_SHORT:
      defineMethod(READ_STRING_SHORT());
      break;
    case OP_BUILD_LIST: {
      ObjList *list = newList(vm.klass.list);
      uint8_t itemCount = READ_BYTE();

      push(OBJ_VAL(list));
      for (int i = itemCount; i > 0; i--) {
        pushToList(list, peek(i));
      }
      pop();

      while (itemCount-- > 0) {
        pop();
      }

      push(OBJ_VAL(list));
      break;
    }
    case OP_BUILD_LIST_SHORT: {
      ObjList *list = newList(vm.klass.list);
      uint16_t itemCount = READ_SHORT();

      push(OBJ_VAL(list));
      for (int i = itemCount; i > 0; i--) {
        pushToList(list, peek(i));
      }
      pop();

      while (itemCount-- > 0) {
        pop();
      }

      push(OBJ_VAL(list));
      break;
    }
    case OP_BUILD_MAP: {
      ObjMap *map = newMap(vm.klass.map);
      uint8_t itemCount = READ_BYTE();

      vm.keep = (Obj *)map;
      for (int i = itemCount - 1; i > 0; i -= 2) {
        tableSet(&map->items, AS_STRING(peek(i)), peek(i - 1));
      }
      vm.stackTop -= itemCount;

      push(OBJ_VAL(map));
      vm.keep = NULL;
      break;
    }
    case OP_BUILD_MAP_SHORT: {
      ObjMap *map = newMap(vm.klass.map);
      uint16_t itemCount = READ_SHORT();

      vm.keep = (Obj *)map;
      for (int i = itemCount - 1; i > 0; i -= 2) {
        tableSet(&map->items, AS_STRING(peek(i)), peek(i - 1));
      }
      vm.stackTop -= itemCount;

      push(OBJ_VAL(map));
      vm.keep = NULL;
      break;
    }
    case OP_INDEX_SUBSCR: {
      if (IS_STRING(peek(0))) {
        ObjString *key = AS_STRING(pop());
        if (!IS_MAP(peek(0))) {
          runtimeError("Can only key into a map.");
          return INTERPRET_RUNTIME_ERROR;
        }
        ObjMap *map = AS_MAP(pop());
        Value value;
        if (tableGet(&map->items, key, &value)) {
          push(value);
          break;
        }
        runtimeError("Key is not a valid member of the map.");
        return INTERPRET_RUNTIME_ERROR;
      }
      if (!IS_NUMBER(peek(0))) {
        runtimeError("Index is not a valid number or string.");
        return INTERPRET_RUNTIME_ERROR;
      }
      double index = AS_NUMBER(pop());

      if (IS_LIST(peek(0))) {
        ObjList *list = AS_LIST(pop());

        if (!isValidListIndex(list, index)) {
          runtimeError("List index out of range.");
          return INTERPRET_RUNTIME_ERROR;
        }

        push(indexFromList(list, index));
      } else if (IS_STRING(peek(0))) {
        ObjString *string = AS_STRING(pop());

        if (index > string->length - 1 || index < 0) {
          runtimeError("String index out of range.");
          return INTERPRET_RUNTIME_ERROR;
        }

        push(OBJ_VAL(
            copyString(string->chars + (int)index, index, &vm.strings)));
      } else {
        runtimeError("Invalid type to index into.");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_STORE_SUBSCR: {
      Value item = pop();
      if (IS_STRING(peek(0))) {
        ObjString *str = AS_STRING(pop());
        if (!IS_MAP(peek(0))) {
          runtimeError("Map key is not a string.");
          return INTERPRET_RUNTIME_ERROR;
        }
        ObjMap *map = AS_MAP(pop());
        tableSet(&map->items, str, item);
        push(item);
        break;
      }
      if (!IS_NUMBER(peek(0))) {
        runtimeError("List index is not a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      int index = AS_NUMBER(pop());

      if (IS_LIST(peek(0))) {
        ObjList *list = AS_LIST(pop());

        if (!isValidListIndex(list, index)) {
          runtimeError("List index out of range.");
          return INTERPRET_RUNTIME_ERROR;
        }

        storeToList(list, index, item);
        push(item);
      } else {
        runtimeError("Invalid type to update by index.");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_IN: {
      if (IS_CLOSURE(peek(0))) {
        ObjClosure *closure = AS_CLOSURE(peek(0));
        int i = 0;
        push(NUMBER_VAL((double)i));
        push(peek(1));
        if (!call(closure, 0)) {
          return INTERPRET_RUNTIME_ERROR;
        }
        frame = &vm.frames[vm.frameCount - 1];
      } else if (IS_CLOSURE(peek(1)) && IS_NUMBER(peek(0))) {
        ObjClosure *closure = AS_CLOSURE(peek(1));
        int i = (int)AS_NUMBER(peek(0));
        i += 1;
        pop();
        push(NUMBER_VAL((double)i));
        push(peek(1));
        if (!call(closure, 0)) {
          return INTERPRET_RUNTIME_ERROR;
        }
        frame = &vm.frames[vm.frameCount - 1];
      } else if (IS_LIST(peek(0))) {
        ObjList *list = AS_LIST(peek(0));
        int i = 0;
        push(NUMBER_VAL((double)i));
        if (!isValidListIndex(list, i)) {
          pop();
          push(NIL_VAL);
          break;
        }
        push(indexFromList(list, i));
      } else if (IS_LIST(peek(1)) && IS_NUMBER(peek(0))) {
        ObjList *list = AS_LIST(peek(1));
        int i = (int)AS_NUMBER(peek(0));
        i += 1;
        if (!isValidListIndex(list, i)) {
          pop();
          push(NIL_VAL);
          break;
        }
        pop();
        push(NUMBER_VAL((double)i));
        push(indexFromList(list, i));
      } else if (IS_STRING(peek(0))) {
        ObjString *string = AS_STRING(peek(0));
        int i = 0;
        push(NUMBER_VAL((double)i));
        if (string->length == 0) {
          pop();
          push(NIL_VAL);
          break;
        }
        ObjString *ch = copyString(string->chars, 1, &vm.strings);
        ch->klass = string->klass;
        push(OBJ_VAL(ch));
      } else if (IS_STRING(peek(1)) && IS_NUMBER(peek(0))) {
        ObjString *string = AS_STRING(peek(1));
        int i = (int)AS_NUMBER(peek(0));
        i += 1;
        if (i > string->length - 1) {
          pop();
          push(NIL_VAL);
          break;
        }
        pop();
        push(NUMBER_VAL((double)i));
        ObjString *ch = copyString(string->chars + i, 1, &vm.strings);
        ch->klass = string->klass;
        push(OBJ_VAL(ch));
      } else if (IS_NIL(peek(0))) {
        break;
      } else {
        runtimeError("Only functions, strings and lists can be used after in.");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    }
  }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_CONSTANT_SHORT
#undef READ_STRING
#undef READ_STRING_SHORT
#undef BINARY_OP
#undef BITWISE_OP
#undef MATH_OP
}

InterpretResult interpret(const char *source, const char *file) {
  ObjFunction *function = compile(source, file);
  if (function == NULL)
    return INTERPRET_COMPILE_ERROR;

  push(OBJ_VAL(function));
  ObjClosure *closure = newClosure(function);
  pop();
  push(OBJ_VAL(closure));
  call(closure, 0);

  return run();
}
