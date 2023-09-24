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

typedef void (*ParseFn)(Parser* parser);

typedef struct {
  ParseFn    prefix;
  ParseFn    infix;
  Precedence precedence;
} ParseRule;

// miscellaneous helpers
static ParseRule* getRule(TokenType type);
static Token*     last(Parser* parser);
static Token*     this(Parser* parser);
static Token*     next(Parser* parser);
static Value      peekXpr(Parser* parser, int i);
static void       saveNum(Parser* parser, Number val);
static void       saveObj(Parser* parser, void* val);
static void       saveVal(Parser* parser, Value val);
static void       consumeXpr(Parser* parser, size_t n);
static bool       isAtEnd(Parser* parser);
static bool       isAtLineBreak(Parser* parser);
static void       advance(Parser* parser);
static void       consume(Parser* parser, TokenType type, const char* message);
static Symbol*    tokenToSymbol(Token token);

#define saveXpr(p, x)                           \
  _Generic((x),                                 \
           Number:saveNum,                      \
           Value: saveVal,                      \
           default:saveObj)(p, x)

// error helpers
static void error(Parser* parser, const char* message);
static void errorAt(Token* token, Parser* parser, const char* message);
static void errorAtCurrent(Parser* parser, const char* message);

// parse rule declarations
static void number(Parser* parser);
static void atomic(Parser* parser);
static void symbol(Parser* parser);
static void string(Parser* parser);
static void identifier(Parser* parser);
static void unary(Parser* parser);
static void binary(Parser* parser);
static void grouping(Parser* parser);
static void expression(Parser* parser);

// toplevel parse helpers
static void parsePrecedence(Parser* parser, Precedence precedence);

// globals
ParseRule rules[] = {
  [NUMBER_TOKEN]        = { number,     NULL,   NO_PRECEDENCE         },
  [SYMBOL_TOKEN]        = { symbol,     NULL,   NO_PRECEDENCE         },
  [STRING_TOKEN]        = { string,     NULL,   NO_PRECEDENCE         },
  [KEYWORD_TOKEN]       = { NULL,       NULL,   NO_PRECEDENCE         },
  [IDENTIFIER_TOKEN]    = { identifier, NULL,   NO_PRECEDENCE         },
  [TRUE_TOKEN]          = { atomic,     NULL,   NO_PRECEDENCE         },
  [FALSE_TOKEN]         = { atomic,     NULL,   NO_PRECEDENCE         },
  [NUL_TOKEN]           = { atomic,     NULL,   NO_PRECEDENCE         },
  [LPAR_TOKEN]          = { grouping,   NULL,   CALL_PRECEDENCE       },
  [RPAR_TOKEN]          = { NULL,       NULL,   NO_PRECEDENCE         },
  [LARROWS_TOKEN]       = { NULL,       NULL,   NO_PRECEDENCE         },
  [RARROWS_TOKEN]       = { NULL,       NULL,   NO_PRECEDENCE         },
  [LBRACK_TOKEN]        = { NULL,       NULL,   NO_PRECEDENCE         },
  [RBRACE_TOKEN]        = { NULL,       NULL,   NO_PRECEDENCE         },
  [DO_TOKEN]            = { NULL,       NULL,   NO_PRECEDENCE         },
  [END_TOKEN]           = { NULL,       NULL,   NO_PRECEDENCE         },
  [COMMA_TOKEN]         = { NULL,       NULL,   NO_PRECEDENCE         },
  [DOT_TOKEN]           = { identifier, binary, ACCESSOR_PRECEDENCE   },
  [EQUAL_TOKEN]         = { identifier, binary, ASSIGNMENT_PRECEDENCE },
  [MATCH_TOKEN]         = { identifier, binary, ASSIGNMENT_PRECEDENCE },
  [COLON_COLON_TOKEN]   = { identifier, binary, ASSIGNMENT_PRECEDENCE },
  [OR_TOKEN]            = { identifier, binary, OR_PRECEDENCE         },
  [AND_TOKEN]           = { identifier, binary, AND_PRECEDENCE        },
  [EQUAL_EQUAL_TOKEN]   = { identifier, binary, EQUALITY_PRECEDENCE   },
  [NOT_EQUAL_TOKEN]     = { identifier, binary, EQUALITY_PRECEDENCE   }, 
  [LESS_THAN_TOKEN]     = { identifier, binary, COMPARISON_PRECEDENCE },
  [GREATER_THAN_TOKEN]  = { identifier, binary, COMPARISON_PRECEDENCE },
  [LESS_EQUAL_TOKEN]    = { identifier, binary, COMPARISON_PRECEDENCE },
  [GREATER_EQUAL_TOKEN] = { identifier, binary, COMPARISON_PRECEDENCE },
  [PLUS_TOKEN]          = { identifier, binary, TERM_PRECEDENCE       },
  [MINUS_TOKEN]         = { identifier, binary, TERM_PRECEDENCE       },
  [BAR_TOKEN]           = { identifier, binary, TERM_PRECEDENCE       },
  [MUL_TOKEN]           = { identifier, binary, FACTOR_PRECEDENCE     },
  [DIV_TOKEN]           = { identifier, binary, FACTOR_PRECEDENCE     },
  [REM_TOKEN]           = { identifier, binary, FACTOR_PRECEDENCE     },
  [APOSTROPHE_TOKEN]    = { unary,      NULL,   UNARY_PRECEDENCE      },
  [NOT_TOKEN]           = { unary,      NULL,   UNARY_PRECEDENCE      },
  [ERROR_TOKEN]         = { NULL,       NULL,   NO_PRECEDENCE         },
  [EOF_TOKEN]           = { NULL,       NULL,   NO_PRECEDENCE         }
};

static ParseRule* getRule(TokenType type) {
  return &rules[type];
}

// misc helpers implementations
static Token* last(Parser* parser) {
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

static Value peekXpr(Parser* parser, int i) {
  if (i < 0)
    i += parser->subExpressions.count;

  assert(i > 0 && i < (int)parser->subExpressions.count);
  return parser->subExpressions.data[i];
}

static void consumeXpr(Parser* parser, size_t n) {
  popValuesN(&parser->subExpressions, n);
}

static void saveNum(Parser* parser, Number val) {
  saveVal(parser, TAG_NUM(val));
}

static void saveObj(Parser* parser, void* val) {
  saveVal(parser, TAG_OBJ(val));
}

static void saveVal(Parser* parser, Value val) {
  writeValues(&parser->subExpressions, val);
}

static bool isAtEnd(Parser* parser) {
  return this(parser)->type == EOF_TOKEN;
}

static bool isAtLineBreak(Parser* parser) {
  return parser->offset > 0 && last(parser)->lineNo < this(parser)->lineNo;
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

static Symbol* tokenToSymbol(Token token) {
  bool  isSymbol   = token.type == SYMBOL_TOKEN;

  char buffer[token.length+!isSymbol];
  buffer[token.length-isSymbol] = '\0';
  memcpy(buffer, token.start+isSymbol, token.length-isSymbol);
  return getSymbol(buffer);
}

// error helper implementations
static void error(Parser* parser, const char* message) {
  errorAt(last(parser), parser, message);
}

static void errorAt(Token* token, Parser* parser, const char* message) {
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
}

static void errorAtCurrent(Parser* parser, const char* message) {
  errorAt(this(parser), parser, message);
}

// parse rule implementations
static void number(Parser* parser) {
  Token token = *this(parser);

  // copy token value
  char buffer[token.length+1];
  buffer[token.length] = '\0';
  memcpy(buffer, token.start, token.length);

  // interpret as float64
  saveXpr(parser, strtod(buffer, NULL));
}

static void atomic(Parser* parser) {
  Token token = *this(parser);
  Value val;

  if (token.type == TRUE_TOKEN)
    val = TRUE_VAL;

  else if (token.type == FALSE_TOKEN)
    val = FALSE_VAL;

  else if (token.type == NUL_TOKEN)
    val = NUL_VAL;

  else {
    val = NOTHING_VAL;
    errorAtCurrent(parser, "Unreadable atomic token.");
  }

  if (val != NOTHING_VAL)
    saveXpr(parser, val);
}

static void symbol(Parser* parser) {
  Token token    = *last(parser);
  Symbol* symbol = tokenToSymbol(token);
  saveXpr(parser, symbol);
}

static void string(Parser* parser) {
  Token token = *last(parser);

  // copy token value, ommitting opening and closing '"'
  char buffer[token.length-1];
  buffer[token.length-2] = '\0';
  memcpy(buffer, token.start+1, token.length-2);
  saveXpr(parser, newString(buffer, token.length-2));
}

static void identifier(Parser* parser) {
  Token token = *last(parser);

  // copy token value
  char buffer[token.length+1];
  buffer[token.length] = '\0';
  memcpy(buffer, token.start, token.length-1);
  Symbol* name = getSymbol(buffer);
  saveXpr(parser, newTriple(TAG_OBJ(name), EMPTY_LIST(), NUL_VAL));
}

static void unary(Parser* parser) {
  Symbol* head = tokenToSymbol(*last(parser));
  parsePrecedence(parser, UNARY_PRECEDENCE);
  List*  subx = newList1(peekXpr(parser, -1));
  Tuple* ast  = newTriple(TAG_OBJ(head), EMPTY_LIST(), TAG_OBJ(subx));
  consumeXpr(parser, 1);
  saveXpr(parser, ast);
}

static void binary(Parser* parser) {
  Token token = *last(parser);
  Symbol* head = tokenToSymbol(token);
  ParseRule* rule = getRule(token.type);
  parsePrecedence(parser, (Precedence)(rule->precedence + 1));
  List* subx = newList2(peekXpr(parser, -2), peekXpr(parser, -1));
  Tuple* ast = newTriple(TAG_OBJ(head), EMPTY_LIST(), TAG_OBJ(subx));
  consumeXpr(parser, 2);
  saveXpr(parser, ast);
}

static void grouping(Parser* parser) {
  expression(parser);
  consume(parser, RPAR_TOKEN, "Expect ')' after expression.");
}

static void expression(Parser* parser) {
  parsePrecedence(parser, ASSIGNMENT_PRECEDENCE);
}

static void parsePrecedence(Parser* parser, Precedence precedence) {
  advance(parser);
  ParseFn prefixRule = getRule(last(parser)->type)->prefix;

  if (prefixRule == NULL)
    error(parser, "Expect expression.");

  else {
    prefixRule(parser);

    while (precedence <= getRule(this(parser)->type)->precedence) {
      advance(parser);
      ParseFn infixRule = getRule(last(parser)->type)->infix;
      infixRule(parser);
    }
  }
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

Value parse(Parser* parser, Scanner* source) {
  initParser(parser, source);
  expression(parser);
  Value out = peekXpr(parser, -1);
  consumeXpr(parser, 1);
  return out;
}
