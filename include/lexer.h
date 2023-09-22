#ifndef rascal_lexer_h
#define rascal_lexer_h

#include "common.h"

/* types and procedures for lexing input (initial processing). */

// token type
typedef enum {
  NUMBER_TOKEN,
  IDENTIFIER_TOKEN,
  ATOM_TOKEN,
  LPAR_TOKEN,
  RPAR_TOKEN,
  PLUS_TOKEN,
  MINUS_TOKEN,
  MUL_TOKEN,
  DIV_TOKEN,
  EXP_TOKEN,
  COMMA_TOKEN,
  EOF_TOKEN
} TokenType;

typedef struct {
  char*     value;
  TokenType type;
  size_t    start;
  size_t    stop;
} Token;

typedef struct {
  char*  source;
  size_t count;
  size_t offset;
  Token  current;
  Token  previous;
} Scanner;

#include "tpl/declare.h"
ARRAY_TYPE(Tokens, Token);

#endif
