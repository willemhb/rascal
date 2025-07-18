#ifndef rl_lang_lex_h
#define rl_lang_lex_h

/**
 *
 * Code for tokenizing Rascal data.
 *
 **/

// headers --------------------------------------------------------------------
#include "lang/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
typedef enum {
  // single character tokens --------------------------------------------------
  TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, TOKEN_COMMA,        TOKEN_DOT,
  TOKEN_MINUS,      TOKEN_PLUS,        TOKEN_SLASH,        TOKEN_STAR,
  TOKEN_EQUAL,      TOKEN_LESS,        TOKEN_GREATER,      TOKEN_PERCENT,
  /* TOKEN_AT, */

  // two character tokens -----------------------------------------------------
  TOKEN_EQUAL_EQUAL,   TOKEN_SLASH_EQUAL, TOKEN_LESS_EQUAL,
  TOKEN_GREATER_EQUAL, TOKEN_LESS_LESS,   TOKEN_GREATER_GREATER,

  // keywords -----------------------------------------------------------------
  TOKEN_FUN,     TOKEN_IF,  TOKEN_ELIF, TOKEN_ELSE,  TOKEN_VAR, TOKEN_VAL,
  TOKEN_DO,      TOKEN_END, TOKEN_AND,  TOKEN_OR,    TOKEN_NOT, TOKEN_REQUIRE,
  TOKEN_PROVIDE, TOKEN_AS,  TOKEN_TRUE, TOKEN_FALSE, TOKEN_NUL,

  // references ---------------------------------------------------------------
  TOKEN_IDENTIFIER,

  // literals -----------------------------------------------------------------
  TOKEN_SYMBOL, TOKEN_STRING, TOKEN_GLYPH, TOKEN_NUMBER,

  // sentinels ----------------------------------------------------------------
  TOKEN_EOF, TOKEN_ERROR,
} TokenType;

typedef struct {
  TokenType type;
  char*     value;
  int       length;
  int       line;
} Token;

typedef struct {
  Port* ios;
  int   line;
  int   offset;  // position in buffer
  char* buffer;
} Scanner;

// globals --------------------------------------------------------------------
extern Scanner Lexer;

// function prototypes --------------------------------------------------------
void  free_scanner(Scanner* scn);
void  init_scanner(Scanner* scn, Port* ios);
Token make_token(Scanner* scn, TokenType type, int length);


// initialization -------------------------------------------------------------
void toplevel_init_lang_lex(void);

#endif
