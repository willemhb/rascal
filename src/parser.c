#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "parser.h"

// internal API
// C types
// precedence levels
typedef enum {
  NO_PRECEDENCE,
  ASSIGNMENT_PRECEDENCE,
  OR_PRECEDENCE,
  AND_PRECEDENCE,
  EQUALITY_PRECEDENCE,
  COMPARISON_PRECEDENCE,
  TERM_PRECEDENCE,
  FACTOR_PRECEDENCE,
  UNARY_PRECEDENCE,
  CALL_PRECEDENCE,
  ACCESSOR_PRECEDENCE,
  PRIMARY_PRECEDENCE
} Precedence;

typedef Value (*ParseFn)(Parser* parser, Scanner* source);

typedef struct {
  ParseFn    prefix;
  ParseFn    infix;
  Precedence precedence;
} ParseRule;

// parse rule declarations
static Value number(Parser* parser, Scanner* source);
static Value symbol(Parser* parser, Scanner* source);
static Value string(Parser* parser, Scanner* source);
static Value keyword(Parser* parser, Scanner* source);
static Value identifier(Parser* parser, Scanner* source);
static Value atomic(Parser* parser, Scanner* source);
static Value grouping(Parser* parser, Scanner* source);
static Value infix(Parser* parser, Scanner* source);
static Value prefix(Parser* parser, Scanner* source);

// globals
ParseRule rules[] = {
  [NUMBER_TOKEN]     = { number,     NULL, NO_PRECEDENCE },
  [SYMBOL_TOKEN]     = { symbol,     NULL, NO_PRECEDENCE },
  [STRING_TOKEN]     = { string,     NULL, NO_PRECEDENCE },
  [KEYWORD_TOKEN]    = { keyword,    NULL, NO_PRECEDENCE },
  [IDENTIFIER_TOKEN] = { identifier, NULL, NO_PRECEDENCE },
  [TRUE_TOKEN]       = { atomic,     NULL, NO_PRECEDENCE },
  [FALSE_TOKEN]      = { atomic,     NULL, NO_PRECEDENCE },
  [NIL_TOKEN]        = { atomic,     NULL, NO_PRECEDENCE },
  [LPAR_TOKEN]       = { grouping,   NULL, NO_PRECEDENCE },
  [RPAR_TOKEN]       = { NULL,       NULL, NO_PRECEDENCE },
  [ERROR_TOKEN]      = { NULL,       NULL, NO_PRECEDENCE },
  [EOF_TOKEN]        = { NULL,       NULL, NO_PRECEDENCE }
};

// parse rule implementations
static Value number(Parser* parser, Scanner* source) {
  (void)source;

  Token token = parser->current;

  assert(token.type == NUMBER_TOKEN);

  // copy token value
  char buffer[token.length+1],* end;
  buffer[token.length] = '\0';
  memcpy(buffer, token.start, token.length);

  // interpret as float64
  Number val = strtod(buffer, &end);
  assert(*end == '\0');
  return TAG_NUM(val);
}

static Value symbol(Parser* parser, Scanner* source) {
  (void)source;

  Token token = parser->current;

  assert(token.type == SYMBOL_TOKEN);

  // copy token value, ommiting ':'
  char buffer[token.length];
  buffer[token.length-1] = '\0';
  memcpy(buffer, token.start+1, token.length-1);

  Symbol* val = getSymbol(buffer);

  return TAG_OBJ(val);
}



// external API
void initParser(Parser* parser) {
  parser->offset     = 0;
  parser->hadError   = false;
  parser->panicMode  = false;
}
