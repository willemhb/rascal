#ifndef rascal_parser_h
#define rascal_parser_h

#include "common.h"
#include "value.h"
#include "scanner.h"

/**
 *
 * Rascal syntactic grammar (for reference. See scanner.h for lexical grammar):
 * 
 * expression  -> assignment ;
 * assignment  -> accessor ( ( "=" | "->" ) logical_or )? | logical_or ;
 * logical_or  -> logical_and ( "or" logical_and )* ;
 * logical_and -> equality ( "and" equality )* ;
 * equality    -> comparison ( ( "==" | "!=" ) comparison )? ;
 * comparison  -> term ( ( ">" | "<" | ">=" | "<=" ) term )? ;
 * term        -> factor ( ( "+" | "-" ) factor )* ;
 * factor      -> unary ( ( "/" | "*" ) unary )* ;
 * unary       -> ( "not" | "-" ) unary | call ;
 * call        -> accessor
                | accessor "(" arguments? ")"
                | accessor arguments
                | accessor arguments? do-block ;
 * arguments   -> expressions ( "," keywords )*  | keywords ;
 * expressions -> expression ( "," expression )* ;
 * keywords    -> keyword expression ( "," keyword expression )* ;
 * do-block    -> "do" expression+ ( identifier expression+ )* "end" ;
 * accessor    -> primary ( "." primary )*
 * primary     -> number
 *              | symbol
 *              | identifier
 *              | string
 *              | bits
 *              | list
 *              | map
 *              | tuple
 *              | grouping ;
 * bits        -> "<<" ( number ( "," number )* )? ">>" ;
 * list        -> "[" ( keywords | expressions )? "]" ;
 * map         -> "{" ( keywords | pairs )? "}" ;
 * tuple       -> "(" ")" | "(" expression "," expressions? ")" ;
 * grouping    -> "(" expression ")" ;
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
