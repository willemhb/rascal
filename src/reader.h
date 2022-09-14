#ifndef rascal_reader_h
#define rascal_reader_h

typedef enum
  {
    // single-character tokens
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACK, TOKEN_RIGHT_BRACK,
    TOKEN_COMMA,      TOKEN_DOT,
    TOKEN_MINUS,      TOKEN_PLUS,
    TOKEN_SEMICOLON,  TOKEN_SLASH,
    TOKEN_STAR,

    // one-or-two character tokens
    TOKEN_BANG,       TOKEN_SLASH_EQUAL,
    TOKEN_EQUAL,      TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,       TOKEN_LESS_EQUAL,
    TOKEN_LEFT_ARROW, TOKEN_RIGHT_ARROW,

    // atoms
    TOKEN_SYMBOL,     TOKEN_KEYWORD,
    TOKEN_STRING,     TOKEN_NUMBER,

    // literals
    TOKEN_NIL,        TOKEN_TRUE,
    TOKEN_FALSE,

    // special forms
    TOKEN_AND,   TOKEN_ELSE, TOKEN_FUN,
    TOKEN_IF,    TOKEN_OR,   TOKEN_PRINT,
    TOKEN_DO,    TOKEN_MAC,  TOKEN_QUOTE,
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

void initScanner( const char *source );
Token scanToken( void );

// misc macros
#define rascal_fallthrough __attribute__((fallthrough))

#endif
