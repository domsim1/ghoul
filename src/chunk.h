#ifndef ghoul_chunk_h
#define ghoul_chunk_h

#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_CONSTANT_SHORT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_GET_LOCAL,
  OP_GET_LOCAL_SHORT,
  OP_SET_LOCAL,
  OP_SET_LOCAL_SHORT,
  OP_GET_GLOBAL,
  OP_GET_GLOBAL_SHORT,
  OP_DEFINE_GLOBAL,
  OP_DEFINE_GLOBAL_SHORT,
  OP_SET_GLOBAL,
  OP_SET_GLOBAL_SHORT,
  OP_GET_UPVALUE,
  OP_GET_UPVALUE_SHORT,
  OP_SET_UPVALUE,
  OP_SET_UPVALUE_SHORT,
  OP_GET_PROPERTY,
  OP_GET_PROPERTY_SHORT,
  OP_SET_PROPERTY,
  OP_SET_PROPERTY_SHORT,
  OP_GET_SUPER,
  OP_GET_SUPER_SHORT,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_BANG_EQUAL,
  OP_GREATER_EQUAL,
  OP_LESS_EQUAL,
  OP_ADD,
  OP_CONCAT,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_MOD,
  OP_EXPONENTIATION,
  OP_NOT,
  OP_NEGATE,
  OP_BITWISE_NOT,
  OP_BITWISE_AND,
  OP_BITWISE_OR,
  OP_BITWISE_XOR,
  OP_BITWISE_LEFT_SHIFT,
  OP_BITWISE_RIGHT_SHIFT,
  OP_PRINT,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_LOOP,
  OP_CALL,
  OP_CALL_SHORT,
  OP_INVOKE,
  OP_INVOKE_SHORT,
  OP_SUPER_INVOKE,
  OP_SUPER_INVOKE_SHORT,
  OP_CLOSURE,
  OP_CLOSURE_SHORT,
  OP_CLOSE_UPVALUE,
  OP_RETURN,
  OP_CLASS,
  OP_CLASS_SHORT,
  OP_INHERIT,
  OP_PROPERTY,
  OP_PROPERTY_SHORT,
  OP_BUILD_LIST,
  OP_BUILD_LIST_SHORT,
  OP_BUILD_MAP,
  OP_BUILD_MAP_SHORT,
  OP_INDEX_SUBSCR,
  OP_STORE_SUBSCR,
  OP_IN,
} OpCode;

typedef struct {
  int count;
  int capacity;
  uint8_t *code;
  int *lines;
  const char *file;
  ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk, const char *file);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
int addConstant(Chunk *chunk, Value value);
void writeConstant(Chunk *chunk, Value value, int line, const char *file);
int getLine(Chunk *chunk, int line);
const char *getFileName(Chunk *chunk);

#endif
