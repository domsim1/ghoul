#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "memory.h"
#include "vm.h"

void initChunk(Chunk *chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lines = NULL;
  chunk->file = NULL;
  initValueArray(&chunk->constants);
}

void freeChunk(Chunk *chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int, chunk->lines, chunk->capacity);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line, const char *file) {
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code =
        GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    chunk->file = GROW_ARRAY(char *, chunk->file, oldCapacity, chunk->capacity);
  }

  chunk->file[chunk->count] = (char *)file;
  chunk->lines[chunk->count] = line;
  chunk->code[chunk->count] = byte;
  chunk->count++;
}

int getLine(Chunk *chunk, int line) { return chunk->lines[line]; }
const char *getLineFileName(Chunk *chunk, int line) {
  return chunk->file[line];
}

void writeConstant(Chunk *chunk, Value value, int line, const char *file) {
  int valueIndex = addConstant(chunk, value);
  if (valueIndex > 255) {
    if (valueIndex > 65535) {
      exit(1);
    }
    writeChunk(chunk, OP_CONSTANT_SHORT, line, file);
    writeChunk(chunk, (valueIndex >> 8) & 0xFF, line, file);
    writeChunk(chunk, valueIndex & 0xFF, line, file);
    return;
  }
  writeChunk(chunk, OP_CONSTANT, line, file);
  writeChunk(chunk, valueIndex, line, file);
}

int addConstant(Chunk *chunk, Value value) {
  push(value);
  writeValueArray(&chunk->constants, value);
  pop();
  return chunk->constants.count - 1;
}
