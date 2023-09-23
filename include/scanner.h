#ifndef rascal_scanner_h
#define rascal_scanner_h

#include "common.h"

/**
 * types and procedures for lexing input (initial processing).
 *
 * Rascal lexical grammar (for reference. See parser.h for syntactic grammar):
 *
 * number     -> digit ( "." digit )? ;
 * symbol     -> ":" identifier ;
 * keyword    -> identifier ":" ;
 * string     -> '"' character* '"' ;
 * identifier -> ( alpha | digit | sep | punct | op )+ ;
 * alpha      -> "a" ... "z" | "A" ... "Z" ;
 * digit      -> "0" ... "9" ;
 * sep        -> "-" | "_" ;
 * punct      -> "!" | "?" ;
 * op         -> "=" | "<" | ">"
 *            |  "+" | "-" | "*" | "/"
 *            |  "^" | "&" | "$" | "%" ;
 **/

// token type
typedef enum {
  // atoms
  NUMBER_TOKEN, STRING_TOKEN, SYMBOL_TOKEN, KEYWORD_TOKEN,
  IDENTIFIER_TOKEN, TRUE_TOKEN, FALSE_TOKEN, NIL_TOKEN,

  // delimiters
  LPAR_TOKEN,    RPAR_TOKEN,
  LARROWS_TOKEN, RARROWS_TOKEN,
  LBRACK_TOKEN,  RBRACK_TOKEN,
  LBRACE_TOKEN,  RBRACE_TOKEN,
  DO_TOKEN,      END_TOKEN,
  COMMA_TOKEN,   DOT_TOKEN,

  // operators
  // assignment
  EQUAL_TOKEN, MATCH_TOKEN,

  // logical
  OR_TOKEN, AND_TOKEN, NOT_TOKEN,

  // comparison
  LESS_THAN_TOKEN,   GREATER_THAN_TOKEN,
  EQUAL_EQUAL_TOKEN, NOT_EQUAL_TOKEN,
  LESS_EQUAL_TOKEN,  GREATER_EQUAL_TOKEN,

  // mathematics
  PLUS_TOKEN, MINUS_TOKEN, MUL_TOKEN, DIV_TOKEN, REM_TOKEN,

  // misc
  EOF_TOKEN, ERROR_TOKEN
} TokenType;

typedef struct {
  const char* start;
  size_t      length;
  TokenType   type;
  int         lineNo;
} Token;

#include "tpl/declare.h"
ARRAY_TYPE(Tokens, Token);

struct Scanner {
  char*  start;
  char*  current;
  int    lineNo;
  bool   hadError;
  Tokens tokens;
};

// scanner API
void  initScanner(Scanner* scanner, char* source);
void  freeScanner(Scanner* scanner);
Token peekToken(Scanner* scanner, int i);

// lexer API
bool lexInput(char* source);

#endif
