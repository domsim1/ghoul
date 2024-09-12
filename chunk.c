#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "memory.h"
#include "vm.h"

void initChunk(Chunk *chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lineCount = 0;
  chunk->lineCapacity = 0;
  chunk->lines = NULL;
  initValueArray(&chunk->constants);
}

void freeChunk(Chunk *chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int, chunk->lines, chunk->capacity);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code =
        GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
  }

  if (chunk->lineCapacity < chunk->lineCount + 3) {
    int oldCapacity = chunk->lineCapacity;
    chunk->lineCapacity = GROW_CAPACITY(oldCapacity);
    chunk->lines =
        GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->lineCapacity);
    if (chunk->lineCount == 0) {
      chunk->lines[chunk->lineCount] = 0;
      chunk->lines[chunk->lineCount + 1] = 0;
    }
  }

  if (chunk->lines[chunk->lineCount] != line) {
    chunk->lineCount += 2;
    chunk->lines[chunk->lineCount] = line;
    chunk->lines[chunk->lineCount + 1] = 0;
  }
  chunk->lines[chunk->lineCount + 1] += 1;

  chunk->code[chunk->count] = byte;
  chunk->count++;
}

int getLine(Chunk *chunk, int line) {
  for (int i = 0; i < chunk->lineCount + 1; i += 2) {
    line -= chunk->lines[i + 1];
    if (line < 0) {
      return chunk->lines[i];
    }
  }
  exit(1);
}

void writeConstant(Chunk *chunk, Value value, int line) {
  int valueIndex = addConstant(chunk, value);
  if (valueIndex > 255) {
    if (valueIndex > 65535) {
      exit(1);
    }
    writeChunk(chunk, OP_CONSTANT_SHORT, line);
    writeChunk(chunk, (valueIndex >> 8) & 0xFF, line);
    writeChunk(chunk, valueIndex & 0xFF, line);
    return;
  }
  writeChunk(chunk, OP_CONSTANT, line);
  writeChunk(chunk, valueIndex, line);
}

int addConstant(Chunk *chunk, Value value) {
  push(value);
  writeValueArray(&chunk->constants, value);
  pop();
  return chunk->constants.count - 1;
}
