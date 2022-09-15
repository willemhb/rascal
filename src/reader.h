#ifndef rascal_reader_h
#define rascal_reader_h

#include "object.h"
#include "table.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
  } Precedence;

typedef enum
  {
    // single character tokens.
    TOKEN_LEFT_PAREN,  TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACK,  TOKEN_RIGHT_BRACK,
    TOKEN_LEFT_BRACE,  TOKEN_RIGHT_BRACE,

    TOKEN_COMMA,       TOKEN_DOT,
    TOKEN_PLUS,        TOKEN_STAR,
    TOKEN_EQUALS,      TOKEN_MINUS,

    // one- or two-character tokens.
    TOKEN_LESS,        TOKEN_LESS_EQUAL,
    TOKEN_LEFT_ARROW,

    TOKEN_GREATER,     TOKEN_GREATER_EQUAL,
    TOKEN_RIGHT_ARROW,

    TOKEN_SLASH,       TOKEN_SLASH_EQUALS,
    
    TOKEN_PERCENT,     TOKEN_PERCENT_LEFT_BRACE,

    // atomic tokens.
    TOKEN_ATOM,        TOKEN_SYMBOL,
    TOKEN_NUMBER,      TOKEN_STRING,
    
    // keywords.
    
  } TokenType;

typedef void (*ParseFn)(void);

typedef struct
{
  TokenType   type;
  const char *start;
  int         length, line;
  Value       value;
} Token;

typedef struct
{
  ParseFn    prefix;
  ParseFn    infix;
  Precedence precedence;
} ParseRule;

typedef struct
{
  const char *start;
  const char *current;
  int line;
} Scanner;

typedef struct
{
  Token current;
  Token previous;
  bool  panicMode;
  bool  hadError;
} Parser;

// globals --------------------------------------------------------------------
extern Scanner scanner;
extern Parser  parser;

// forward declarations -------------------------------------------------------
void  initScanner( Scanner *scanner, const char *source );
void  initParser( Parser *parser );

Value readExpression( Scanner *scanner );

#endif
