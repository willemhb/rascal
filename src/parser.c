#include <stdlib.h>
#include <stdio.h>
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

typedef Value (*ParseFn)(Parser* parser);

typedef struct {
  ParseFn    prefix;
  ParseFn    infix;
  Precedence precedence;
} ParseRule;

// miscellaneous helpers
static bool isAtEnd(Parser* parser);

// error helpers
static Value error(Parser* parser, const char* message);
static Value errorAt(Token* token, Parser* parser, const char* message);
static Value errorAtCurrent(Parser* parser, const char* message);

// parse rule declarations
static Value number(Parser* parser);
static Value atomic(Parser* parser);
static Value symbol(Parser* parser);
static Value string(Parser* parser);
static Value keyword(Parser* parser);
static Value identifier(Parser* parser);
static Value grouping(Parser* parser);
static Value list(Parser* parser);
static Value bits(Parser* parser);
static Value infix(Parser* parser);
static Value prefix(Parser* parser);
static Value expression(Parser* parser);

// globals
ParseRule rules[] = {
  [NUMBER_TOKEN]        = { number,     NULL, NO_PRECEDENCE         },
  [SYMBOL_TOKEN]        = { symbol,     NULL, NO_PRECEDENCE         },
  [STRING_TOKEN]        = { string,     NULL, NO_PRECEDENCE         },
  [KEYWORD_TOKEN]       = { keyword,    NULL, NO_PRECEDENCE         },
  [IDENTIFIER_TOKEN]    = { identifier, NULL, NO_PRECEDENCE         },
  [TRUE_TOKEN]          = { atomic,     NULL, NO_PRECEDENCE         },
  [FALSE_TOKEN]         = { atomic,     NULL, NO_PRECEDENCE         },
  [NIL_TOKEN]           = { atomic,     NULL, NO_PRECEDENCE         },
  [LPAR_TOKEN]          = { grouping,   NULL, NO_PRECEDENCE         },
  [RPAR_TOKEN]          = { NULL,       NULL, NO_PRECEDENCE         },
  [LARROWS_TOKEN]       = { NULL,       NULL, NO_PRECEDENCE         },
  [RARROWS_TOKEN]       = { NULL,       NULL, NO_PRECEDENCE         },
  [LBRACK_TOKEN]        = { NULL,       NULL, NO_PRECEDENCE         },
  [RBRACE_TOKEN]        = { NULL,       NULL, NO_PRECEDENCE         },
  [DO_TOKEN]            = { NULL,       NULL, NO_PRECEDENCE         },
  [END_TOKEN]           = { NULL,       NULL, NO_PRECEDENCE         },
  [COMMA_TOKEN]         = { NULL,       NULL, NO_PRECEDENCE         },
  [DOT_TOKEN]           = { NULL,       NULL, ACCESSOR_PRECEDENCE   },
  [EQUAL_TOKEN]         = { NULL,       NULL, ASSIGNMENT_PRECEDENCE },
  [MATCH_TOKEN]         = { NULL,       NULL, ASSIGNMENT_PRECEDENCE },
  [OR_TOKEN]            = { NULL,       NULL, OR_PRECEDENCE         },
  [AND_TOKEN]           = { NULL,       NULL, AND_PRECEDENCE        },
  [NOT_TOKEN]           = { NULL,       NULL, UNARY_PRECEDENCE      },
  [EQUAL_EQUAL_TOKEN]   = { NULL,       NULL, EQUALITY_PRECEDENCE   },
  [NOT_EQUAL_TOKEN]     = { NULL,       NULL, EQUALITY_PRECEDENCE   }, 
  [LESS_THAN_TOKEN]     = { NULL,       NULL, COMPARISON_PRECEDENCE },
  [GREATER_THAN_TOKEN]  = { NULL,       NULL, COMPARISON_PRECEDENCE },
  [LESS_EQUAL_TOKEN]    = { NULL,       NULL, COMPARISON_PRECEDENCE },
  [GREATER_EQUAL_TOKEN] = { NULL,       NULL, COMPARISON_PRECEDENCE },
  [PLUS_TOKEN]          = { NULL,       NULL, TERM_PRECEDENCE       },
  [MINUS_TOKEN]         = { NULL,       NULL, TERM_PRECEDENCE       },
  [MUL_TOKEN]           = { NULL,       NULL, FACTOR_PRECEDENCE     },
  [DIV_TOKEN]           = { NULL,       NULL, FACTOR_PRECEDENCE     },
  [REM_TOKEN]           = { NULL,       NULL, FACTOR_PRECEDENCE     },
  [ERROR_TOKEN]         = { NULL,       NULL, NO_PRECEDENCE         },
  [EOF_TOKEN]           = { NULL,       NULL, NO_PRECEDENCE         }
};

static ParseRule* getRule(TokenType type) {
  return &rules[type];
}

// misc helpers implementations
static Token* previous(Parser* parser) {
  if ( parser->offset == 0 )
    return NULL;

  return &parser->source->tokens.data[parser->offset-1];
}

static Token* this(Parser* parser) {
  return &parser->source->tokens.data[parser->offset];
}

static Token* next(Parser* parser) {
  if (isAtEnd(parser))
    return this(parser);

  return &parser->source->tokens.data[parser->offset+1];
}

static bool isAtEnd(Parser* parser) {
  return this(parser)->type == EOF_TOKEN;
}

static void advance(Parser* parser) {
  while (!isAtEnd(parser)) { // don't advance past end of input
    parser->offset++;
    if (this(parser)->type != ERROR_TOKEN)
      break;

    errorAtCurrent(parser, this(parser)->start);
  }
}

static void consume(Parser* parser, TokenType type, const char* message) {
  if (this(parser)->type == type)
    advance(parser);

  else
    errorAtCurrent(parser, message);
}

// error helper implementations
static Value error(Parser* parser, const char* message) {
  return errorAt(previous(parser), parser, message);
}
static Value errorAt(Token* token, Parser* parser, const char* message) {
  if (!parser->panicMode) {  
    parser->panicMode = true;
    fprintf(stderr, "[line %d] Error", token->lineNo);

    if (token->type == EOF_TOKEN) {
      fprintf(stderr, " at end");
    } else if (token->type == ERROR_TOKEN) {
      // Nothing.
    } else {
      fprintf(stderr, " at '%.*s'", (int)token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser->hadError = true;
  }

  return NOTHING_VAL;
}

static Value errorAtCurrent(Parser* parser, const char* message) {
  return errorAt(this(parser), parser, message);
}

// parse rule implementations
static Value number(Parser* parser) {
  Token token = *this(parser);

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

static Value atomic(Parser* parser) {
  Token token = *this(parser);
  Value val;

  if (token.type == TRUE_TOKEN)
    val = TRUE_VAL;

  else if (token.type == FALSE_TOKEN)
    val = FALSE_VAL;

  else if (token.type == NIL_TOKEN)
    val = NIL_VAL;

  else
    val = errorAtCurrent(parser, "Unreadable atomic token.");

  return val;
}

static Value symbol(Parser* parser) {
  Token token = *this(parser);

  assert(token.type == SYMBOL_TOKEN);

  // copy token value, ommiting initial ':'
  char buffer[token.length];
  buffer[token.length-1] = '\0';
  memcpy(buffer, token.start+1, token.length-1);

  Symbol* val = getSymbol(buffer);

  return TAG_OBJ(val);
}

static Value string(Parser* parser) {
  Token token = *this(parser);

  // copy token value, ommitting opening and closing '"'
  char buffer[token.length-1];
  buffer[token.length-2] = '\0';
  memcpy(buffer, token.start+1, token.length-2);

  Bits* val = newString(buffer, token.length-2);

  return TAG_OBJ(val);
}

static Value keyword(Parser* parser) {
  Token token = *this(parser);

  // copy token value, ommiting terminal ':'
  char buffer[token.length];
  buffer[token.length-1] = '\0';
  memcpy(buffer, token.start+1, token.length-1);

  Symbol* val = getSymbol(buffer);

  return TAG_OBJ(val);
}

static Value grouping(Parser* parser) {
  Value val = expression(parser);
}

// external API
void initParser(Parser* parser, Scanner* source) {
  parser->source     = source;
  parser->offset     = 0;
  parser->hadError   = false;
  parser->panicMode  = false;
  initValues(&parser->subExpressions);
}

void freeParser(Parser* parser) {
  freeValues(&parser->subExpressions);
}
