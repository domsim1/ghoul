#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "native/native.h"
#include "scanner.h"
#include "table.h"
#include "vm.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
} Parser;

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,
  PREC_OR,
  PREC_AND,
  PREC_BITWISE_OR,
  PREC_BITWISE_XOR,
  PREC_BITWISE_AND,
  PREC_EQUALITY,
  PREC_COMPARISION,
  PREC_BITWIES_SHIFT,
  PREC_TERM,
  PREC_FACTOR,
  PREC_EXPONENTIATION,
  PREC_UNARY,
  PREC_CALL,
  PREC_PRIMARY,
  PREC_SUBSCRIPT
} Precedence;

typedef void (*ParseFn)(bool canAssign);

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

typedef struct {
  Token name;
  int depth;
  bool isCaptured;
} Local;

typedef struct {
  uint16_t index;
  bool isLocal;
} Upvalue;

typedef enum {
  TYPE_FUNCTION,
  TYPE_INITIALIZER,
  TYPE_METHOD,
  TYPE_SCRIPT,
} FunctionType;

typedef enum {
  TYPE_BREAK,
  TYPE_CONTINUE,
} FlowType;

typedef struct {
  int location;
  FlowType type;
} FlowStatement;

FlowStatement flowStatements[UINT8_COUNT];

typedef struct LoopContext {
  struct LoopContext *previous;
  int statementCount;
} LoopContext;

typedef struct Compiler {
  struct Compiler *enclosing;
  ObjFunction *function;
  FunctionType type;

  Local locals[UINT16_COUNT];
  int localCount;
  Upvalue upvalues[UINT16_COUNT];
  int scopeDepth;
  const char *file;
} Compiler;

typedef struct ClassCompiler {
  struct ClassCompiler *enclosing;
  bool hasSuperclass;
} ClassCompiler;

Parser parser;
Compiler *current = NULL;
ClassCompiler *currentClass = NULL;
LoopContext *currentLoop = NULL;
bool forceAssignment = false;

static char actualpath[PATH_MAX + 1];

static Chunk *currentChunk() { return &current->function->chunk; }

static void errorAt(Token *token, const char *message) {
  if (parser.panicMode)
    return;
  parser.panicMode = true;
  fprintf(stderr, "[line %d of %s] Error", token->line, current->file);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}

static void error(const char *message) { errorAt(&parser.previous, message); }

static void errorAtCurrent(const char *message) {
  errorAt(&parser.current, message);
}

static char *readFile(const char *path) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    errorAt(&parser.previous, "Could not open file");
    return NULL;
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(fileSize + 1);
  if (buffer == NULL) {
    errorAt(&parser.previous, "Not enough memory to read file.");
    return NULL;
  }
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    errorAt(&parser.previous, "Could not read file.");
    return NULL;
  }
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

static void advance() {
  parser.previous = parser.current;

  for (;;) {
    parser.current = scanToken();
    if (parser.current.type != TOKEN_ERROR)
      break;

    errorAtCurrent(parser.current.start);
  }
}

static void consume(TokenType type, const char *message) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  errorAtCurrent(message);
}

static bool check(TokenType type) { return parser.current.type == type; }

static bool match(TokenType type) {
  if (!check(type))
    return false;
  advance();
  return true;
}

static void emitByte(uint8_t byte) {
  writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
  emitByte(byte1);
  emitByte(byte2);
}

static void emitShort(uint16_t bytes) {
  emitBytes((bytes >> 8) & 0xff, bytes & 0xff);
}

static void emitIndex(uint16_t bytes) {
  if (bytes > UINT8_MAX) {
    emitShort(bytes);
  } else {
    emitByte((uint8_t)bytes);
  }
}

static void emitLoop(int loopStart) {
  emitByte(OP_LOOP);

  int offset = currentChunk()->count - loopStart + 2;
  if (offset > UINT16_MAX)
    error("Loop body too large.");

  emitByte((offset >> 8) & 0xff);
  emitByte(offset & 0xff);
}

static int emitJump(uint8_t instruction) {
  emitByte(instruction);
  emitByte(0xff);
  emitByte(0xff);
  return currentChunk()->count - 2;
}

static void emitReturn() {
  if (current->type == TYPE_INITIALIZER) {
    emitBytes(OP_GET_LOCAL, 0);
  } else {
    emitByte(OP_NIL);
  }

  emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
  int constant = addConstant(currentChunk(), value);
  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    // this should never be reached
    return 0;
  }

  return (uint8_t)constant;
}

static uint16_t makeConstantShort(Value value) {
  int constant = addConstant(currentChunk(), value);
  if (constant > UINT16_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return (uint16_t)constant;
}

static void emitConstant(Value value) {
  if (currentChunk()->count > UINT8_MAX) {
    uint16_t constant = makeConstantShort(value);
    emitByte(OP_CONSTANT_SHORT);
    emitShort(constant);
  } else {
    emitBytes(OP_CONSTANT, makeConstant(value));
  }
}

static void patchJump(int offset) {
  int jump = currentChunk()->count - offset - 2;

  if (jump > UINT16_MAX) {
    error("Too much code to jump over.");
  }

  currentChunk()->code[offset] = (jump >> 8) & 0xff;
  currentChunk()->code[offset + 1] = jump & 0xff;
}

static void startLoop(LoopContext *loopContext) {
  loopContext->statementCount = 0;
  loopContext->previous = currentLoop;
  currentLoop = loopContext;
}
static void endLoop() { currentLoop = currentLoop->previous; }

static void patchFlowJumps(int loopStart) {

  while (currentLoop->statementCount > 0) {
    FlowStatement *statement = &flowStatements[--currentLoop->statementCount];
    if (statement->type == TYPE_BREAK) {
      patchJump(statement->location);
    } else if (statement->type == TYPE_CONTINUE) {
      int loopOffset = statement->location - loopStart + 2;
      if (loopOffset > UINT16_MAX) {
        error("Too much code to jump over.");
      }
      currentChunk()->code[statement->location] = (loopOffset >> 8) & 0xff;
      currentChunk()->code[statement->location + 1] = loopOffset & 0xff;
    }
  }
}

static void initCompiler(Compiler *compiler, FunctionType type,
                         const char *file) {
  compiler->enclosing = current;
  compiler->function = NULL;
  compiler->type = type;
  compiler->localCount = 0;
  compiler->scopeDepth = 0;
  compiler->function = newFunction(file);
  current = compiler;
  if (type != TYPE_SCRIPT) {
    if (parser.previous.type == TOKEN_COLON) {
      current->function->name = copyString("λ", 2, &vm.strings);
    } else {
      current->function->name = copyString(parser.previous.start,
                                           parser.previous.length, &vm.strings);
    }
  }

  int pathLength = strlen(file);
  uint32_t hash = hashString(file, pathLength);
  char *heapChars = ALLOCATE(char, pathLength + 1);
  memcpy(heapChars, file, pathLength);
  heapChars[pathLength] = '\0';
  ObjString *realFilePath = allocateString(heapChars, pathLength, hash,
                                           &vm.useStrings, vm.klass.string);
  compiler->file = realFilePath->chars;

  Local *local = &current->locals[current->localCount++];
  local->depth = 0;
  local->isCaptured = false;
  if (type != TYPE_FUNCTION) {
    local->name.start = "this";
    local->name.length = 4;
  } else {
    local->name.start = "";
    local->name.length = 0;
  }
}

static ObjFunction *endCompiler() {
  emitReturn();
  ObjFunction *function = current->function;

#ifdef DEBUG_PRINT_CODE
  if (!parser.hadError) {
    disassembleChunk(currentChunk(), function->name != NULL
                                         ? function->name->chars
                                         : "<script>");
  }

  current->scopeDepth--;
#endif

  current = current->enclosing;
  return function;
}

static void beginScope() { current->scopeDepth++; }

static void endScope() {
  current->scopeDepth--;

  while (current->localCount > 0 &&
         current->locals[current->localCount - 1].depth > current->scopeDepth) {
    if (current->locals[current->localCount - 1].isCaptured) {
      emitByte(OP_CLOSE_UPVALUE);
    } else {
      emitByte(OP_POP);
    }
    current->localCount--;
  }
}

static void expression();
static void statement();
static void declaration();
static ParseRule *getRule(TokenType type);
static void parsePrecedence(Precedence precedence);
static void function(FunctionType type);

static bool isAssignment(uint8_t *binaryOp) {
  TokenType operatorType = parser.current.type;

  switch (operatorType) {
  case TOKEN_EQUAL:
    *binaryOp = OP_NIL;
    break;
  case TOKEN_BITWISE_AND_EQUAL:
    *binaryOp = OP_BITWISE_AND;
    break;
  case TOKEN_BITWISE_OR_EQUAL:
    *binaryOp = OP_BITWISE_OR;
    break;
  case TOKEN_BITWISE_XOR_EQUAL:
    *binaryOp = OP_BITWISE_XOR;
    break;
  case TOKEN_BITWISE_RIGHT_SHIFT_EQUAL:
    *binaryOp = OP_BITWISE_RIGHT_SHIFT;
    break;
  case TOKEN_BITWISE_LEFT_SHIFT_EQUAL:
    *binaryOp = OP_BITWISE_LEFT_SHIFT;
    break;
  case TOKEN_PLUS_EQUAL:
    *binaryOp = OP_ADD;
    break;
  case TOKEN_MINUS_EQUAL:
    *binaryOp = OP_SUBTRACT;
    break;
  case TOKEN_STAR_EQUAL:
    *binaryOp = OP_MULTIPLY;
    break;
  case TOKEN_SLASH_EQUAL:
    *binaryOp = OP_DIVIDE;
    break;
  case TOKEN_PERCENTAGE_EQUAL:
    *binaryOp = OP_MOD;
    break;
  case TOKEN_STAR_STAR_EQUAL:
    *binaryOp = OP_EXPONENTIATION;
    break;
  default:
    return false;
  }
  advance();
  return true;
}

static uint16_t identifierConstant(Token *name) {
  if (currentChunk()->count > UINT8_MAX) {
    return makeConstantShort(
        OBJ_VAL(copyString(name->start, name->length, &vm.strings)));
  }
  return makeConstant(
      OBJ_VAL(copyString(name->start, name->length, &vm.strings)));
}

static bool identifierEqual(Token *a, Token *b) {
  if (a->length != b->length)
    return false;
  return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler *compiler, Token *name) {
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local *local = &compiler->locals[i];
    if (identifierEqual(name, &local->name)) {
      if (local->depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }
  return -1;
}

static int addUpvalue(Compiler *compiler, uint16_t index, bool isLocal) {
  int upvalueCount = compiler->function->upvalueCount;

  for (int i = 0; i < upvalueCount; i++) {
    Upvalue *upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->isLocal == isLocal) {
      return i;
    }
  }

  if (upvalueCount >= UINT16_COUNT) {
    error("Too many closure variables in function.");
    return 0;
  }

  compiler->upvalues[upvalueCount].isLocal = isLocal;
  compiler->upvalues[upvalueCount].index = index;
  return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Compiler *compiler, Token *name) {
  if (compiler->enclosing == NULL)
    return -1;

  int local = resolveLocal(compiler->enclosing, name);
  if (local != -1) {
    compiler->enclosing->locals[local].isCaptured = true;
    return addUpvalue(compiler, (uint16_t)local, true);
  }

  int upvalue = resolveUpvalue(compiler->enclosing, name);
  if (upvalue != -1) {
    return addUpvalue(compiler, (uint16_t)upvalue, false);
  }

  return -1;
}

static void namedVariable(Token name, bool canAssign) {
  uint8_t getOp, setOp, binaryOp;
  int arg = resolveLocal(current, &name);

  if (arg != -1) {
    if (arg > UINT8_MAX) {
      getOp = OP_GET_LOCAL_SHORT;
      setOp = OP_SET_LOCAL_SHORT;
    } else {
      getOp = OP_GET_LOCAL;
      setOp = OP_SET_LOCAL;
    }
  } else if ((arg = resolveUpvalue(current, &name)) != -1) {
    if (arg > UINT8_MAX) {
      getOp = OP_GET_UPVALUE_SHORT;
      setOp = OP_SET_UPVALUE_SHORT;
    } else {
      getOp = OP_GET_UPVALUE;
      setOp = OP_SET_UPVALUE;
    }
  } else {
    arg = identifierConstant(&name);
    if (arg > UINT8_MAX) {
      getOp = OP_GET_GLOBAL_SHORT;
      setOp = OP_SET_GLOBAL_SHORT;
    } else {
      getOp = OP_GET_GLOBAL;
      setOp = OP_SET_GLOBAL;
    }
  }

  if (canAssign && isAssignment(&binaryOp)) {
    if (binaryOp == OP_NIL) {
      expression();
      emitByte(setOp);
      emitIndex(arg);
    } else {
      emitByte(getOp);
      emitIndex(arg);
      expression();
      emitByte(binaryOp);
      emitByte(setOp);
      emitIndex(arg);
    }
  } else if (forceAssignment) {
    emitByte(setOp);
    emitIndex(arg);
  } else {
    emitByte(getOp);
    emitIndex(arg);
  }
}

static void variable(bool canAssign) {
  namedVariable(parser.previous, canAssign);
}

static Token syntheticToken(const char *text) {
  Token token;
  token.start = text;
  token.length = (int)strlen(text);
  return token;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

static void this_(bool canAssign) {
  if (currentClass == NULL) {
    error("Can't use 'this' outside of a class.");
    return;
  }
  variable(false);
}

static void binary(bool canAssign) {
  TokenType operatorType = parser.previous.type;
  ParseRule *rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (operatorType) {
  case TOKEN_BANG_EQUAL:
    emitByte(OP_BANG_EQUAL);
    break;
  case TOKEN_EQUAL_EQUAL:
    emitByte(OP_EQUAL);
    break;
  case TOKEN_GREATER:
    emitByte(OP_GREATER);
    break;
  case TOKEN_GREATER_EQUAL:
    emitByte(OP_GREATER_EQUAL);
    break;
  case TOKEN_LESS:
    emitByte(OP_LESS);
    break;
  case TOKEN_LESS_EQUAL:
    emitByte(OP_LESS_EQUAL);
    break;
  case TOKEN_BANG:
    emitByte(OP_NOT);
    break;
  case TOKEN_PLUS:
    emitByte(OP_ADD);
    break;
  case TOKEN_PLUS_PLUS:
    emitByte(OP_CONCAT);
    break;
  case TOKEN_MINUS:
    emitByte(OP_SUBTRACT);
    break;
  case TOKEN_STAR:
    emitByte(OP_MULTIPLY);
    break;
  case TOKEN_SLASH:
    emitByte(OP_DIVIDE);
    break;
  case TOKEN_PERCENTAGE:
    emitByte(OP_MOD);
    break;
  case TOKEN_STAR_STAR:
    emitByte(OP_EXPONENTIATION);
    break;
  case TOKEN_BITWISE_AND:
    emitByte(OP_BITWISE_AND);
    break;
  case TOKEN_BITWISE_OR:
    emitByte(OP_BITWISE_OR);
    break;
  case TOKEN_BITWISE_XOR:
    emitByte(OP_BITWISE_XOR);
    break;
  case TOKEN_BITWISE_LEFT_SHIFT:
    emitByte(OP_BITWISE_LEFT_SHIFT);
    break;
  case TOKEN_BITWISE_RIGHT_SHIFT:
    emitByte(OP_BITWISE_RIGHT_SHIFT);
    break;
  default:
    return;
  }
}

static uint16_t argumentList() {
  uint16_t argCount = 0;
  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      expression();
      if (argCount == UINT16_MAX) {
        error("Can't have more than 65535 arguments.");
      }
      argCount++;
    } while (match(TOKEN_COMMA));
  }
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
  return argCount;
}

static void call(bool canAssign) {
  uint16_t argCount = argumentList();
  if (argCount > UINT8_MAX) {
    emitByte(OP_CALL_SHORT);
  } else {
    emitByte(OP_CALL);
  }
  emitIndex(argCount);
}

static void dot(bool canAssign) {
  consume(TOKEN_IDENTIFIER, "Expect property identifier after '.'.");
  uint16_t name = identifierConstant(&parser.previous);

  uint8_t binaryOp;
  if (canAssign && isAssignment(&binaryOp)) {
    if (binaryOp == OP_NIL) {
      expression();
      if (name > UINT8_MAX) {
        emitByte(OP_SET_PROPERTY_SHORT);
      } else {
        emitByte(OP_SET_PROPERTY);
      }
      emitIndex(name);
    } else {
      emitBytes(currentChunk()->code[currentChunk()->count - 2],
                currentChunk()->code[currentChunk()->count - 1]);
      if (name > UINT8_MAX) {
        emitByte(OP_GET_PROPERTY_SHORT);
      } else {
        emitByte(OP_GET_PROPERTY);
      }
      emitIndex(name);
      expression();
      emitByte(binaryOp);
      if (name > UINT8_MAX) {
        emitByte(OP_SET_PROPERTY_SHORT);
      } else {
        emitByte(OP_SET_PROPERTY);
      }
      emitIndex(name);
    }
  } else if (match(TOKEN_LEFT_PAREN)) {
    uint16_t argCount = argumentList();
    if (name > UINT8_MAX || argCount > UINT8_MAX) {
      emitByte(OP_INVOKE_SHORT);
      emitShort(name);
      emitShort(argCount);
    } else {
      emitByte(OP_INVOKE);
      emitByte((uint8_t)name);
      emitByte((uint8_t)argCount);
    }
  } else {
    if (name > UINT8_MAX) {
      emitByte(OP_GET_PROPERTY_SHORT);
    } else {
      emitByte(OP_GET_PROPERTY);
    }
    emitIndex(name);
  }
}

static void literal(bool canAssign) {
  switch (parser.previous.type) {
  case TOKEN_FALSE:
    emitByte(OP_FALSE);
    break;
  case TOKEN_NIL:
    emitByte(OP_NIL);
    break;
  case TOKEN_TRUE:
    emitByte(OP_TRUE);
    break;
  default:
    exit(1); // unreachable
    return;
  }
}

static void grouping(bool canAssign) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool canAssign) {
  double value = strtod(parser.previous.start, NULL);
  emitConstant(NUMBER_VAL(value));
}

static void string(bool canAssign) {
  emitConstant(OBJ_VAL(copyEscString(parser.previous.start + 1,
                                     parser.previous.length - 2, &vm.strings,
                                     vm.klass.string)));
}

static void list(bool canAssign) {
  int itemCount = 0;
  if (!check(TOKEN_RIGHT_BRACKET)) {
    do {
      if (check(TOKEN_RIGHT_BRACKET)) {
        // trailing comma
        break;
      }

      parsePrecedence(PREC_OR);

      if (itemCount >= UINT16_COUNT) {
        error("Cannot have more than 65535 items in a list literal.");
      }
      itemCount++;
    } while (match(TOKEN_COMMA));
  }

  consume(TOKEN_RIGHT_BRACKET, "Expect ']' after list literal.");

  if (itemCount > UINT8_MAX) {
    emitByte(OP_BUILD_LIST_SHORT);
  } else {
    emitByte(OP_BUILD_LIST);
  }
  emitIndex((uint16_t)itemCount);
}

static void map(bool canAssign) {
  int itemCount = 0;
  if (!check(TOKEN_RIGHT_BRACE)) {
    do {
      if (check(TOKEN_RIGHT_BRACE)) {
        // trailing comma
        break;
      }

      consume(TOKEN_STRING, "Expect string for map key.");
      string(false);
      consume(TOKEN_COLON, "Expect ':' after key.");
      parsePrecedence(PREC_OR);

      if (itemCount >= UINT16_COUNT) {
        error("Cannot have more than 65535 keys and values in a map literal.");
      }
      itemCount += 2;
    } while (match(TOKEN_COMMA));
  }

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after map literal.");

  if (itemCount > UINT8_MAX) {
    emitByte(OP_BUILD_MAP_SHORT);
  } else {
    emitByte(OP_BUILD_MAP);
  }
  emitIndex((uint16_t)itemCount);
}

static void subscript(bool canAssign) {
  parsePrecedence(PREC_OR);
  consume(TOKEN_RIGHT_BRACKET, "Expect ']' after index.");

  uint8_t binaryOp;
  if (canAssign && isAssignment(&binaryOp)) {
    if (binaryOp == OP_NIL) {
      expression();
      emitByte(OP_STORE_SUBSCR);
    } else {
      emitBytes(currentChunk()->code[currentChunk()->count - 4],
                currentChunk()->code[currentChunk()->count - 3]);
      emitBytes(currentChunk()->code[currentChunk()->count - 4],
                currentChunk()->code[currentChunk()->count - 3]);
      emitByte(OP_INDEX_SUBSCR);
      expression();
      emitByte(binaryOp);
      emitByte(OP_STORE_SUBSCR);
    }
  } else {
    emitByte(OP_INDEX_SUBSCR);
  }
  return;
}

static void super_(bool canAssign) {
  if (currentClass == NULL) {
    error("Can't use 'super' outside of a class.");
  } else if (!currentClass->hasSuperclass) {
    error("Can't use 'super' in a class with no superclass.");
  }
  consume(TOKEN_DOT, "Expect '.' after 'super'.");
  consume(TOKEN_IDENTIFIER, "Expect superclass method identifier.");
  uint16_t name = identifierConstant(&parser.previous);

  namedVariable(syntheticToken("this"), false);

  if (match(TOKEN_LEFT_PAREN)) {
    uint16_t argCount = argumentList();
    namedVariable(syntheticToken("super"), false);
    if (name > UINT8_MAX || argCount > UINT8_MAX) {
      emitByte(OP_SUPER_INVOKE_SHORT);
      emitShort(name);
      emitShort(argCount);
    } else {
      emitBytes(OP_SUPER_INVOKE, (uint8_t)name);
      emitByte((uint8_t)argCount);
    }
  } else {
    namedVariable(syntheticToken("super"), false);
    if (name > UINT8_MAX) {
      emitByte(OP_GET_SUPER_SHORT);
    } else {
      emitByte(OP_GET_SUPER);
    }
    emitIndex(name);
  }
}

static void lambda(bool canAssign) { function(TYPE_FUNCTION); }

static void and_(bool canAssign) {
  int endJump = emitJump(OP_JUMP_IF_FALSE);

  emitByte(OP_POP);
  parsePrecedence(PREC_AND);

  patchJump(endJump);
}

static void or_(bool canAssign) {
  int elseJump = emitJump(OP_JUMP_IF_FALSE);
  int endJump = emitJump(OP_JUMP);

  patchJump(elseJump);
  emitByte(OP_POP);

  parsePrecedence(PREC_OR);
  patchJump(endJump);
}

static void unary(bool canAssign) {
  TokenType operatorType = parser.previous.type;

  parsePrecedence(PREC_UNARY);

  switch (operatorType) {
  case TOKEN_BANG:
    emitByte(OP_NOT);
    break;
  case TOKEN_MINUS:
    emitByte(OP_NEGATE);
    break;
  case TOKEN_BITWISE_NOT:
    emitByte(OP_BITWISE_NOT);
    break;
  default:
    exit(1);
    return;
  }
}
#pragma GCC diagnostic pop

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, call, PREC_CALL},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE] = {map, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACKET] = {list, subscript, PREC_SUBSCRIPT},
    [TOKEN_RIGHT_BRACKET] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, dot, PREC_CALL},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_PLUS_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_BITWISE_NOT] = {unary, NULL, PREC_UNARY},
    [TOKEN_BITWISE_AND] = {NULL, binary, PREC_BITWISE_AND},
    [TOKEN_BITWISE_OR] = {NULL, binary, PREC_BITWISE_XOR},
    [TOKEN_BITWISE_XOR] = {NULL, binary, PREC_BITWISE_OR},
    [TOKEN_BITWISE_LEFT_SHIFT] = {NULL, binary, PREC_BITWIES_SHIFT},
    [TOKEN_BITWISE_RIGHT_SHIFT] = {NULL, binary, PREC_BITWIES_SHIFT},
    [TOKEN_BITWISE_AND_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_BITWISE_OR_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_BITWISE_XOR_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOKEN_PERCENTAGE] = {NULL, binary, PREC_FACTOR},
    [TOKEN_PERCENTAGE_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR_STAR] = {NULL, binary, PREC_EXPONENTIATION},
    [TOKEN_STAR_STAR_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_BANG] = {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISION},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISION},
    [TOKEN_LESS] = {NULL, binary, PREC_COMPARISION},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISION},
    [TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},
    [TOKEN_STRING] = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
    [TOKEN_AND] = {NULL, and_, PREC_AND},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = {literal, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, or_, PREC_OR},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {super_, NULL, PREC_NONE},
    [TOKEN_THIS] = {this_, NULL, PREC_NONE},
    [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
    [TOKEN_COLON] = {lambda, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_BREAK] = {NULL, NULL, PREC_NONE},
    [TOKEN_USE] = {NULL, NULL, PREC_NONE},
    [TOKEN_IN] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

static void parsePrecedence(Precedence precedence) {
  advance();
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Expect expression.");
    return;
  }

  bool canAssign = precedence <= PREC_ASSIGNMENT;
  prefixRule(canAssign);

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    infixRule(canAssign);
  }
  uint8_t temp;
  if (canAssign && isAssignment(&temp)) {
    error("Invalid assignment target.");
  }
}

static void addLocal(Token name) {
  if (current->localCount >= UINT16_COUNT) {
    error("Too many local variable in function.");
    return;
  }

  Local *local = &current->locals[current->localCount++];
  local->name = name;
  local->depth = current->scopeDepth;
  local->isCaptured = false;
}

static void declareVariable() {
  if (current->scopeDepth == 0)
    return;

  Token *name = &parser.previous;
  for (int i = current->localCount - 1; i >= 0; i--) {
    Local *local = &current->locals[i];
    if (local->depth != -1 && local->depth < current->scopeDepth) {
      break;
    }

    if (identifierEqual(name, &local->name)) {
      error("Already a variable with this identifier in this scope.");
    }
  }
  addLocal(*name);
}

static uint16_t parseVariable() {
  declareVariable();
  if (current->scopeDepth > 0)
    return 0;

  return identifierConstant(&parser.previous);
}

static void markInitialized() {
  if (current->scopeDepth == 0)
    return;
  current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint16_t global) {
  if (current->scopeDepth > 0) {
    markInitialized();
    return;
  }
  if (global > UINT8_MAX) {
    emitByte(OP_DEFINE_GLOBAL_SHORT);
  } else {
    emitByte(OP_DEFINE_GLOBAL);
  }
  emitIndex(global);
}

static ParseRule *getRule(TokenType type) { return &rules[type]; }

static void expression() { parsePrecedence(PREC_ASSIGNMENT); }

static void block() {
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }
  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(FunctionType type) {
  Compiler compiler;
  initCompiler(&compiler, type, current->file);
  beginScope();

  consume(TOKEN_LEFT_PAREN, "Expect '(' after function identifier.");
  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      current->function->arity++;
      if (current->function->arity > UINT16_MAX) {
        errorAtCurrent("Can't have more than 65535 parameters.");
      }
      if (match(TOKEN_STAR)) {
        current->function->variadic = true;
        consume(TOKEN_IDENTIFIER, "Expect parameter identifier.");
        uint16_t constant = parseVariable();
        defineVariable(constant);
        if (check(TOKEN_COMMA)) {
          errorAtCurrent("Variadic parameter must be the last patameter.");
        }
      } else {
        consume(TOKEN_IDENTIFIER, "Expect parameter identifier.");
        uint16_t constant = parseVariable();
        defineVariable(constant);
      }
    } while (match(TOKEN_COMMA));
  }
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
  consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
  block();

  ObjFunction *function = endCompiler();
  uint16_t constant = makeConstantShort(OBJ_VAL(function));
  if (constant > UINT8_MAX) {
    emitByte(OP_CLOSURE_SHORT);
  } else {
    emitByte(OP_CLOSURE);
  }
  emitIndex(constant);

  for (int i = 0; i < function->upvalueCount; i++) {
    emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
    emitShort(compiler.upvalues[i].index);
  }
}

static void method(bool canInit) {
  uint16_t constant = identifierConstant(&parser.previous);

  FunctionType type = TYPE_METHOD;
  if (canInit && parser.previous.length == 4 &&
      memcmp(parser.previous.start, "init", 4) == 0) {
    type = TYPE_INITIALIZER;
  }
  function(type);
  if (constant > UINT8_MAX) {
    emitByte(OP_PROPERTY_SHORT);
  } else {
    emitByte(OP_PROPERTY);
  }
  emitIndex(constant);
}

static void staticVariable() {
  uint16_t constant = identifierConstant(&parser.previous);
  consume(TOKEN_EQUAL, "Expect assignment after identifier");
  expression();
  if (constant > UINT8_MAX) {
    emitByte(OP_PROPERTY_SHORT);
  } else {
    emitByte(OP_PROPERTY);
  }
  emitIndex(constant);
  consume(TOKEN_SEMICOLON, "Expect ';' after static class assignment.");
}

static bool checkBuitinName(int start, int length, const char *rest,
                            ObjString *name) {
  return name->length == start + length &&
         memcmp(name->chars + start, rest, length) == 0;
}

static bool matchUseBuiltin(ObjString *name) {
  switch (name->chars[0]) {
  case 'M':
    if (checkBuitinName(1, 3, "ath", name)) {
      registerMathNatives();
      return true;
    }
    break;
  case 'R':
    if (checkBuitinName(1, 6, "equest", name)) {
      registerRequestNatives();
      return true;
    } else if (checkBuitinName(1, 1, "L", name)) {
      registerRaylibNatives();
      return true;
    }
    break;
  case 'J':
    if (checkBuitinName(1, 3, "SON", name)) {
      registerJsonNatives();
      return true;
    }
    break;
  }
  return false;
}

static void useStatement() {
  consume(TOKEN_STRING, "Expect file path.");
  Token useFile = parser.previous;

  ObjString *usePath = copyString(parser.previous.start + 1,
                                  parser.previous.length - 2, &vm.strings);

  if (matchUseBuiltin(usePath)) {
    consume(TOKEN_SEMICOLON, "Expect ';' after use statement.");
    return;
  }

  char *resolvePathRes = realpath(usePath->chars, actualpath);
  if (resolvePathRes == NULL) {
    errorAt(&useFile, "Failed to resolve file path.");
    return;
  }

  int pathLength = strlen(actualpath);
  uint32_t hash = hashString(actualpath, pathLength);
  ObjString *interned =
      tableFindString(&vm.useStrings, actualpath, pathLength, hash);

  if (interned != NULL) {
    // already used
    consume(TOKEN_SEMICOLON, "Expect ';' after use statement.");
    return;
  }

  char *heapChars = ALLOCATE(char, pathLength + 1);
  memcpy(heapChars, actualpath, pathLength);
  heapChars[pathLength] = '\0';
  ObjString *realFilePath = allocateString(heapChars, pathLength, hash,
                                           &vm.useStrings, vm.klass.string);

  char *source = readFile(realFilePath->chars);
  if (source == NULL) {
    return;
  }

  Scanner oldScanner = scanner;
  initScanner(source);
  Compiler compiler;
  initCompiler(&compiler, TYPE_SCRIPT, realFilePath->chars);
  advance();
  while (!check(TOKEN_EOF)) {
    declaration();
  }
  ObjFunction *function = endCompiler();

  free(source);
  scanner = oldScanner;
  scanner.current--;
  advance();
  consume(TOKEN_SEMICOLON, "Expect ';' after use statement.");

  uint16_t constant = makeConstantShort(OBJ_VAL(function));
  if (constant > UINT8_MAX) {
    emitByte(OP_CLOSURE_SHORT);
  } else {
    emitByte(OP_CLOSURE);
  }
  emitIndex(constant);

  for (int i = 0; i < function->upvalueCount; i++) {
    emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
    emitShort(compiler.upvalues[i].index);
  }

  emitByte(OP_CALL);
  emitIndex(0);
  emitByte(OP_POP);
}

static void classDeclaration() {
  Token className = parser.previous;
  uint16_t nameConstant = identifierConstant(&parser.previous);
  declareVariable();

  if (nameConstant > UINT8_MAX) {
    emitByte(OP_CLASS_SHORT);
  } else {
    emitByte(OP_CLASS);
  }
  emitIndex(nameConstant);
  defineVariable(nameConstant);

  ClassCompiler classCompiler;
  classCompiler.hasSuperclass = false;
  classCompiler.enclosing = currentClass;
  currentClass = &classCompiler;

  if (match(TOKEN_LESS)) {
    consume(TOKEN_IDENTIFIER, "Expect superclass identifier.");
    variable(false);

    if (identifierEqual(&className, &parser.previous)) {
      error("A class can't inherit from itself.");
    }

    beginScope();
    addLocal(syntheticToken("super"));
    defineVariable(0);

    namedVariable(className, false);
    emitByte(OP_INHERIT);
    classCompiler.hasSuperclass = true;
  }

  namedVariable(className, false);
  consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    consume(TOKEN_IDENTIFIER, "Expect identifier.");
    if (check(TOKEN_LEFT_PAREN)) {
      method(true);
    } else {
      staticVariable();
    }
  }
  consume(TOKEN_RIGHT_BRACE, "Expect '}' before class body.");
  emitByte(OP_POP);

  if (classCompiler.hasSuperclass) {
    endScope();
  }

  currentClass = currentClass->enclosing;
}

static void funDeclaration(uint16_t global) {
  markInitialized();
  function(TYPE_FUNCTION);
  defineVariable(global);
}

static void varDeclaration() {
  consume(TOKEN_IDENTIFIER, "Expect declaration identifier.");

  if (check(TOKEN_LEFT_BRACE) || check(TOKEN_LESS)) {
    classDeclaration();
    return;
  }

  uint16_t global = parseVariable();
  if (check(TOKEN_LEFT_PAREN)) {
    funDeclaration(global);
    return;
  }

  if (match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }

  if (parser.previous.type != TOKEN_RIGHT_BRACE) {
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
  }
  defineVariable(global);
}

static void expressionStatment() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

static void genericFor(Token identifier) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");
  int loopStart = currentChunk()->count;
  emitByte(OP_IN);
  forceAssignment = true;
  namedVariable(identifier, true);
  forceAssignment = false;

  LoopContext loopContext;
  startLoop(&loopContext);

  int exitJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);

  // OP_IN stores values in stack, need to ofset for locals
  current->localCount += 2;
  statement();
  current->localCount -= 2;
  emitLoop(loopStart);

  patchJump(exitJump);
  patchFlowJumps(loopStart);
  endLoop();
  emitByte(OP_POP);
  emitByte(OP_POP);
  endScope();
}

static void forStatement() {
  beginScope();
  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
  if (match(TOKEN_SEMICOLON)) {

  } else if (match(TOKEN_COLON)) {
    consume(TOKEN_IDENTIFIER, "Expect declaration identifier.");
    Token identifier = parser.previous;
    uint16_t global = parseVariable();
    if (match(TOKEN_EQUAL)) {
      expression();
    } else if (match(TOKEN_IN)) {
      emitByte(OP_NIL);
      defineVariable(global);
      genericFor(identifier);
      return;
    } else {
      emitByte(OP_NIL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
  } else {
    expressionStatment();
  }

  int loopStart = currentChunk()->count;
  int exitJump = -1;
  if (!match(TOKEN_SEMICOLON)) {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

    exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
  }
  if (!match(TOKEN_RIGHT_PAREN)) {
    int bodyJump = emitJump(OP_JUMP);
    int incrementStart = currentChunk()->count;
    expression();
    emitByte(OP_POP);
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

    emitLoop(loopStart);
    loopStart = incrementStart;
    patchJump(bodyJump);
  }

  LoopContext loopContext;
  startLoop(&loopContext);

  statement();
  emitLoop(loopStart);

  if (exitJump != -1) {
    patchJump(exitJump);
    emitByte(OP_POP);
  }
  patchFlowJumps(loopStart);

  endLoop();
  endScope();
}

static void printStatement() {
  expression();
  if (parser.previous.type != TOKEN_RIGHT_BRACE) {
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
  }
  emitByte(OP_PRINT);
}

static void returnStatement() {
  if (current->type == TYPE_SCRIPT) {
    error("Can't return from top-level code.");
  }
  if (match(TOKEN_SEMICOLON)) {
    emitReturn();
  } else {
    if (current->type == TYPE_INITIALIZER) {
      error("Can't return a value from an initializer.");
    }
    expression();
    if (parser.previous.type != TOKEN_RIGHT_BRACE) {
      consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
    }
    emitByte(OP_RETURN);
  }
}

static void whileStatment() {
  int loopStart = currentChunk()->count;
  consume(TOKEN_LEFT_PAREN, "Expect '(' after while.");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  LoopContext loopContext;
  startLoop(&loopContext);

  int exitJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();
  emitLoop(loopStart);

  patchJump(exitJump);
  emitByte(OP_POP);

  patchFlowJumps(loopStart);
  endLoop();
}

static void continueStatement() {
  if (currentLoop == NULL) {
    errorAt(&parser.previous, "Can only continue within a loop.");
    consume(TOKEN_SEMICOLON, "Expect ';' after continue.");
    return;
  }
  if (currentLoop->statementCount > UINT8_MAX) {
    errorAt(&parser.previous,
            "Can't have more than 255 flow statements within a single loop.");
  }
  FlowStatement *statement = &flowStatements[currentLoop->statementCount++];
  statement->location = emitJump(OP_LOOP);
  statement->type = TYPE_CONTINUE;
  consume(TOKEN_SEMICOLON, "Expect ';' after continue.");
}

static void breakStatement() {
  if (currentLoop == NULL) {
    errorAt(&parser.previous, "Can only break within a loop.");
    consume(TOKEN_SEMICOLON, "Expect ';' after break.");
    return;
  }
  if (currentLoop->statementCount > UINT8_MAX) {
    errorAt(&parser.previous,
            "Can't have more than 255 flow statements within a single loop.");
  }
  FlowStatement *statement = &flowStatements[currentLoop->statementCount++];
  statement->location = emitJump(OP_JUMP);
  statement->type = TYPE_BREAK;
  consume(TOKEN_SEMICOLON, "Expect ';' after break.");
}

static void ifStatement() {
  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  int thenJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();

  int elseJump = emitJump(OP_JUMP);

  patchJump(thenJump);
  emitByte(OP_POP);

  if (match(TOKEN_ELSE))
    statement();
  patchJump(elseJump);
}

static void synchronize() {
  parser.panicMode = false;

  while (parser.current.type != TOKEN_EOF) {
    if (parser.previous.type == TOKEN_SEMICOLON)
      return;
    switch (parser.current.type) {
    case TOKEN_COLON:
    case TOKEN_FOR:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_PRINT:
    case TOKEN_RETURN:
      return;
    default:; // Nothing to do.
    }

    advance();
  }
}

static void declaration() {
  if (match(TOKEN_COLON)) {
    varDeclaration();
  } else {
    statement();
  }

  if (parser.panicMode)
    synchronize();
}

static void statement() {
  if (match(TOKEN_PRINT)) {
    printStatement();
  } else if (match(TOKEN_FOR)) {
    forStatement();
  } else if (match(TOKEN_IF)) {
    ifStatement();
  } else if (match(TOKEN_RETURN)) {
    returnStatement();
  } else if (match(TOKEN_WHILE)) {
    whileStatment();
  } else if (match(TOKEN_BREAK)) {
    breakStatement();
  } else if (match(TOKEN_CONTINUE)) {
    continueStatement();
  } else if (match(TOKEN_LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else if (match(TOKEN_USE)) {
    useStatement();
  } else {
    expressionStatment();
  }
}

ObjFunction *compile(const char *source, const char *file) {
  initScanner(source);
  Compiler compiler;
  initCompiler(&compiler, TYPE_SCRIPT, file);

  parser.hadError = false;
  parser.panicMode = false;

  advance();
  while (!match(TOKEN_EOF)) {
    declaration();
  }

  ObjFunction *function = endCompiler();
  return parser.hadError ? NULL : function;
}

void markCompilerRoots() {
  Compiler *compiler = current;
  while (compiler != NULL) {
    markObject((Obj *)compiler->function);
    compiler = compiler->enclosing;
  }
}
