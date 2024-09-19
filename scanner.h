#ifndef ghoul_scanner_h
#define ghoul_scanner_h

typedef enum {
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_LEFT_BRACKET,
  TOKEN_RIGHT_BRACKET,
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_MINUS,
  TOKEN_MINUS_EQUAL,
  TOKEN_PLUS,
  TOKEN_PLUS_EQUAL,
  TOKEN_SEMICOLON,
  TOKEN_SLASH,
  TOKEN_SLASH_EQUAL,
  TOKEN_PERCENTAGE,
  TOKEN_PERCENTAGE_EQUAL,
  TOKEN_STAR,
  TOKEN_STAR_EQUAL,
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS,
  TOKEN_LESS_EQUAL,
  TOKEN_BITWISE_NOT,
  TOKEN_BITWISE_AND,
  TOKEN_BITWISE_OR,
  TOKEN_BITWISE_XOR,
  TOKEN_BITWISE_LEFT_SHIFT,
  TOKEN_BITWISE_RIGHT_SHIFT,
  TOKEN_BITWISE_AND_EQUAL,
  TOKEN_BITWISE_OR_EQUAL,
  TOKEN_BITWISE_XOR_EQUAL,
  TOKEN_BITWISE_LEFT_SHIFT_EQUAL,
  TOKEN_BITWISE_RIGHT_SHIFT_EQUAL,
  TOKEN_IDENTIFIER,
  TOKEN_STRING,
  TOKEN_NUMBER,
  TOKEN_AND,
  TOKEN_CLASS,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FOR,
  TOKEN_FUN,
  TOKEN_IF,
  TOKEN_BREAK,
  TOKEN_NIL,
  TOKEN_OR,
  TOKEN_PRINT,
  TOKEN_RETURN,
  TOKEN_SUPER,
  TOKEN_THIS,
  TOKEN_TRUE,
  TOKEN_VAR,
  TOKEN_WHILE,
  TOKEN_ERROR,
  TOKEN_USE,
  TOKEN_EOF
} TokenType;

typedef struct {
  TokenType type;
  const char *start;
  int length;
  int line;
} Token;

typedef struct {
  const char *start;
  const char *current;
  int line;
} Scanner;

extern Scanner scanner;

void initScanner(const char *source);
Token scanToken();

#endif
