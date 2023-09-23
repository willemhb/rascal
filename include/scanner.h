#ifndef rascal_scanner_h
#define rascal_scanner_h

#include "common.h"

/* types and procedures for lexing input (initial processing). */

// token type
typedef enum {
  NUMBER_TOKEN,
  IDENTIFIER_TOKEN,
  SYMBOL_TOKEN,
  LPAR_TOKEN,
  RPAR_TOKEN,
  PLUS_TOKEN,
  MINUS_TOKEN,
  MUL_TOKEN,
  DIV_TOKEN,
  EXP_TOKEN,
  COMMA_TOKEN,
  EOF_TOKEN,
  ERROR_TOKEN
} TokenType;

typedef struct {
  const char* start;
  size_t      length;
  TokenType   type;
  int         lineNo;
} Token;

#include "tpl/declare.h"
ARRAY_TYPE(Tokens, Token);

typedef struct {
  char*  start;
  char*  current;
  int    lineNo;
  bool   hadError;
  Tokens tokens;
} Scanner;

// scanner API
void  initScanner(Scanner* scanner, char* source);
void  freeScanner(Scanner* scanner);
Token peekToken(Scanner* scanner, int i);

// lexer API
bool lexInput(char* source);

#endif
