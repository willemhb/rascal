#ifndef rascal_reader_h
#define rascal_reader_h

#include "value.h"

typedef enum
  {
    // single-character tokens
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACK, TOKEN_RIGHT_BRACK,
    
    TOKEN_COMMA,      TOKEN_MINUS,
    TOKEN_PLUS,       TOKEN_SLASH,
    TOKEN_STAR,

    // one-or-two character tokens
    TOKEN_DOT,        TOKEN_DOT_DOT,
    TOKEN_EQUAL,      TOKEN_SLASH_EQUAL,
    TOKEN_GREATER,    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,       TOKEN_LESS_EQUAL,
    TOKEN_LEFT_ARROW, TOKEN_RIGHT_ARROW,

    // atoms
    TOKEN_SYMBOL,     TOKEN_KEYWORD,
    TOKEN_STRING,     TOKEN_NUMBER,
    TOKEN_NIL,        TOKEN_TRUE,
    TOKEN_FALSE,

    // delimiters
    TOKEN_END,

    // sentinels
    TOKEN_ERROR, TOKEN_EOF
  } TokenType;

typedef struct
{
  TokenType type;
  const char *start;
  int length;
  int line;
} Token;

typedef struct
{
  Token currentToken, previousToken;
  Value currentValue, previousValue;
  
  bool  hadError;
  bool  panicMode;
} Parser;

typedef struct
{
  const char *start;
  const char *current;
  int length;
  int line;
} Scanner;

void initScanner( const char *source );
Token scanToken( void );
Value readExpression( void );

// misc macros
#define rascal_fallthrough __attribute__((fallthrough))

#endif
