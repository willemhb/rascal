#ifndef rascal_parser_h
#define rascal_parser_h

#include "common.h"
#include "value.h"
#include "scanner.h"

/**
 *
 * formal grammar (for reference)
 *
 * EXPRESSION
 *     : ADDITION
 *     ;
 *
 * ADDITION
 *     : ADDITION ('+' | '-') CALL 
 *     | CALL
 *     ;
 *
 * CALL
 *     : MULTIPLICATION
 *     | IDENTIFIER CALL
 *     | IDENTIFIER '(' EXPRESSIONS? ')'
 *     ;
 *
 * MULTIPLICATION
 *     : MULTIPLICATION ('*' | '/') EXPONENTIATION
 *     | EXPONENTIATION
 *     ;
 * 
 * EXPONENTIATION 
 *     : EXPONENTIATION '^' BASIC 
 *     | BASIC
 *     ;
 *
 * BASIC
 *     : number
 *     | symbol
 *     | identifier
 *     | '[' EXPRESSIONS? ']'
 *     | '{' EXPRESSIONS? '}'
 *     | '(' EXPRESSION ')'
 *     ;
 *
 * EXPRESSIONS
 *    : EXPRESSION [',' EXPRESSION ]*
 *
 **/

typedef struct {
  Token    current;
  Token    previous;
  size_t   offset;   // offset within scanner.tokens
  bool     hadError;
  Scanner* scanner;
  Value    expression;
} Parser;

// external API
void initParser(Parser* parser, Scanner* scanner);
bool parseInput(Parser* parser);

#endif
