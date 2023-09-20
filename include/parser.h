#ifndef rascal_parser_h
#define rascal_parser_h

#include "common.h"

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
 *     | IDENTIFIER '(' ')' 
 *     | IDENTIFIER '(' EXPRESSION [',' EXPRESSION]* ')' 
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
 *     | identifier
 *     | atom
 *     | '(' EXPRESSION ')'
 *     ;
 *
 **/



#endif
