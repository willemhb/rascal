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
 *     : ASSIGNMENT
 *     ;
 *
 * ASSIGNMENT
 *     : ASSIGNMENT '=' COMPARISON
 *     | COMPARISON
 *     ;
 *
 * COMPARISON
 *     : COMPARISON ('<' | '>' | '==' | '!=' | '<=' | '>=') ADDITION
 *     | ADDITION
 *     ;
 *
 * ADDITION
 *     : ADDITION ('+' | '-') CALL
 *     | CALL
 *     ;
 *
 * CALL
 *     : MULTIPLICATION
 *     | IDENTIFIER EXPRESSIONS
 *     | IDENTIFIER '(' EXPRESSIONS? ')'
 *     | IDENTIFIER (EXPRESSIONS ',')? KEYWORDS
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
 *     : NUMBER
 *     | SYMBOL
 *     | STRING
 *     | BITS
 *     | IDENTIFIER
 *     | LIST
 *     | MAP
 *     | TUPLE
 *     | '(' EXPRESSION ')'
 *     ;
 *
 * EXPRESSIONS
 *    : EXPRESSION (',' EXPRESSION )*
 *    ;
 *
 * KEYWORDS
 *    : KEYWORD EXPRESSION (',' KEYWORD EXPRESSION)*
 *    ;
 *
 * NUMBER
 *    : DIGIT+ ('.' DIGIT+ )?
 *    ;
 *
 * SYMBOL
 *    : ':' IDENTIFIER
 *    ;
 *
 * STRING
 *    : '"' CHAR* '"'
 *    ;
 *
 * BITS
 *    : '<<' (DIGIT+ (',' DIGIT+)*)? '>>'
 *    ;
 *
 * IDENTIFIER
 *    : IDENTIFIER-CHARACTER+
 *    ;
 *
 * IDENTIFIER-CHARACTER
 *    : ALPHA
 *    | DIGIT
 *    | "+" | "-" | "*" | "/" | "^"
 *    | "!" | "?" | "\" | "@" | "$"
 *    | "$" | "%" | "&" | "|" | "="
 *    | "<" | ">" | "~" | "`"
 *    ;
 *
 * LIST
 *    : '[' EXPRESSIONS? ']'
 *    
 *    
 **/

struct Parser {
  Token    current;
  Token    previous;
  size_t   offset;   // offset within scanner.tokens
  bool     hadError;
  Scanner* scanner;
  Value    expression;
};

// external API
void initParser(Parser* parser, Scanner* scanner);
bool parseInput(Parser* parser);

#endif
