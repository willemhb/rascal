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
static Value      peekXpr(Parser* parser, int i);
static void       saveNum(Parser* parser, Number val);
static void       saveObj(Parser* parser, void* val);
static void       saveVal(Parser* parser, Value val);
static void       consumeXpr(Parser* parser, size_t n);
static bool       isAtEnd(Parser* parser);
static bool       isAtLineBreak(Parser* parser);
static void       incOffset(Parser* parser);
static void       decOffset(Parser* parser);
static void       advance(Parser* parser);
static void       consume(Parser* parser, TokenType type, const char* message);
static Symbol*    tokenToSymbol(Token token);
static bool       check(Parser* parser, TokenType type);
static bool       match(Parser* parser, TokenType type);

#define saveXpr(p, x)                           \
  _Generic((x),                                 \
           Number:saveNum,                      \
           Value: saveVal,                      \
           default:saveObj)(p, x)

// error helpers
static void error(Parser* parser, const char* message);
static void errorAt(Token* token, Parser* parser, const char* message);
static void errorAtCurrent(Parser* parser, const char* message);
static void synchronize(Parser* parser);

// parse rule declarations
static void number(Parser* parser);
static void atomic(Parser* parser);
static void symbol(Parser* parser);
static void string(Parser* parser);
static void identifier(Parser* parser);
static void unary(Parser* parser);
static void quote(Parser* parser);
static void binary(Parser* parser);
static void grouping(Parser* parser);
static void list(Parser* parser);
static void call(Parser* parser);
static void expression(Parser* parser);

// toplevel parse helpers
static size_t arguments(Parser* parser, bool explicit);
static size_t keywords(Parser* parser, TokenType terminal);
static void   parsePrecedence(Parser* parser, Precedence precedence);

// globals
ParseRule rules[] = {
  [NO_TOKEN]            = { NULL,       NULL,   NO_PRECEDENCE         },
  [NUMBER_TOKEN]        = { number,     call,   CALL_PRECEDENCE       },
  [SYMBOL_TOKEN]        = { symbol,     call,   CALL_PRECEDENCE       },
  [STRING_TOKEN]        = { string,     call,   CALL_PRECEDENCE       },
  [KEYWORD_TOKEN]       = { NULL,       NULL,   NO_PRECEDENCE         },
  [IDENTIFIER_TOKEN]    = { identifier, call,   CALL_PRECEDENCE       },
  [TRUE_TOKEN]          = { atomic,     call,   CALL_PRECEDENCE       },
  [FALSE_TOKEN]         = { atomic,     call,   CALL_PRECEDENCE       },
  [NUL_TOKEN]           = { atomic,     call,   CALL_PRECEDENCE       },
  [LPAR_TOKEN]          = { grouping,   call,   CALL_PRECEDENCE       },
  [RPAR_TOKEN]          = { NULL,       NULL,   NO_PRECEDENCE         },
  [LARROWS_TOKEN]       = { NULL,       NULL,   NO_PRECEDENCE         },
  [RARROWS_TOKEN]       = { NULL,       NULL,   NO_PRECEDENCE         },
  [LBRACK_TOKEN]        = { list,       call,   CALL_PRECEDENCE       },
  [RBRACE_TOKEN]        = { NULL,       NULL,   NO_PRECEDENCE         },
  [DO_TOKEN]            = { NULL,       NULL,   NO_PRECEDENCE         },
  [END_TOKEN]           = { NULL,       NULL,   NO_PRECEDENCE         },
  [COMMA_TOKEN]         = { NULL,       NULL,   NO_PRECEDENCE         },
  [DOT_TOKEN]           = { NULL,       binary, ACCESSOR_PRECEDENCE   },
  [EQUAL_TOKEN]         = { NULL,       binary, ASSIGNMENT_PRECEDENCE },
  [MATCH_TOKEN]         = { NULL,       binary, ASSIGNMENT_PRECEDENCE },
  [COLON_COLON_TOKEN]   = { NULL,       binary, ASSIGNMENT_PRECEDENCE },
  [OR_TOKEN]            = { NULL,       binary, OR_PRECEDENCE         },
  [AND_TOKEN]           = { NULL,       binary, AND_PRECEDENCE        },
  [EQUAL_EQUAL_TOKEN]   = { NULL,       binary, EQUALITY_PRECEDENCE   },
  [NOT_EQUAL_TOKEN]     = { NULL,       binary, EQUALITY_PRECEDENCE   }, 
  [LESS_THAN_TOKEN]     = { NULL,       binary, COMPARISON_PRECEDENCE },
  [GREATER_THAN_TOKEN]  = { NULL,       binary, COMPARISON_PRECEDENCE },
  [LESS_EQUAL_TOKEN]    = { NULL,       binary, COMPARISON_PRECEDENCE },
  [GREATER_EQUAL_TOKEN] = { NULL,       binary, COMPARISON_PRECEDENCE },
  [PLUS_TOKEN]          = { NULL,       binary, TERM_PRECEDENCE       },
  [MINUS_TOKEN]         = { NULL,       binary, TERM_PRECEDENCE       },
  [BAR_TOKEN]           = { NULL,       binary, TERM_PRECEDENCE       },
  [MUL_TOKEN]           = { NULL,       binary, FACTOR_PRECEDENCE     },
  [DIV_TOKEN]           = { NULL,       binary, FACTOR_PRECEDENCE     },
  [REM_TOKEN]           = { NULL,       binary, FACTOR_PRECEDENCE     },
  [APOSTROPHE_TOKEN]    = { quote,      call,   UNARY_PRECEDENCE      },
  [NOT_TOKEN]           = { unary,      call,   UNARY_PRECEDENCE      },
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
  if (isAtEnd(parser))
    return last(parser);

  return &parser->source->tokens.data[parser->offset];
}

static Value peekXpr(Parser* parser, int i) {
  if (i < 0)
    i += parser->subXprs.count;

  assert(i >= 0 && i < (int)parser->subXprs.count);
  return parser->subXprs.data[i];
}

static void consumeXpr(Parser* parser, size_t n) {
  popValuesN(&parser->subXprs, n);
}

static void saveNum(Parser* parser, Number val) {
  saveVal(parser, TAG_NUM(val));
}

static void saveObj(Parser* parser, void* val) {
  saveVal(parser, TAG_OBJ(val));
}

static void saveVal(Parser* parser, Value val) {
  writeValues(&parser->subXprs, val);
  #ifdef DEBUG_PARSER
  printValues(stdout, &parser->subXprs);
  #endif
}

static bool isAtEnd(Parser* parser) {
  return parser->offset == parser->source->tokens.count;
}

static bool isAtLineBreak(Parser* parser) {
  return parser->offset > 0 && last(parser)->lineNo < this(parser)->lineNo;
}

static void incOffset(Parser* parser) {
  if (parser->offset < parser->source->tokens.count)
    parser->offset++;
}

static void decOffset(Parser* parser) {
  if (parser->offset > 0)
    parser->offset--;
}

static void advance(Parser* parser) {
  if (isAtEnd(parser))
    return;

  for(;;) {
    incOffset(parser);
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

static bool check(Parser* parser, TokenType type) {
  return this(parser)->type == type;
}

static bool match(Parser* parser, TokenType type) {
  bool out = check(parser, type);

  if (out)
    advance(parser);

  return out;
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

static void synchronize(Parser* parser) {
  parser->panicMode = false;

  while (this(parser)->type != EOF_TOKEN) {
    advance(parser);
  }
}

// parse rule implementations
static void number(Parser* parser) {
  Token token = *last(parser);

  // copy token value
  char buffer[token.length+1];
  buffer[token.length] = '\0';
  memcpy(buffer, token.start, token.length);

  // interpret as float64
  saveXpr(parser, strtod(buffer, NULL));
}

static void atomic(Parser* parser) {
  Token token = *last(parser);
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
  memcpy(buffer, token.start, token.length);
  Symbol* name = getSymbol(buffer);
  saveXpr(parser, newTriple(TAG_OBJ(name), EMPTY_LIST(), NUL_VAL));
}

static void quote(Parser* parser) {
  if (isAtLineBreak(parser)) {
    error(parser, "Expected name given to quote.");
  }

  advance(parser);

  if (isAtEnd(parser)) {
    error(parser, "Expected name given to quote.");
  }

  if (last(parser)->type > DOT_TOKEN && last(parser)->type < EOF_TOKEN)
    identifier(parser);

  else { // ignore
    decOffset(parser);
    expression(parser);
  }
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
  // has to distinguish between the empty tuple, simple grouping, and non-empty tuples
  if (match(parser, RPAR_TOKEN))
    saveXpr(parser, EMPTY_TUPLE());

  else {
    expression(parser);

    if (match(parser, COMMA_TOKEN)) {
      size_t n = 1;

      while (!match(parser, RPAR_TOKEN)) {
        if (isAtEnd(parser)) {
          errorAtCurrent(parser, "Unmatched '(' to close Tuple literal.");
          return;
        }

        expression(parser);
        n++;
        if (!check(parser, RPAR_TOKEN))
          consume(parser, COMMA_TOKEN, "Expect ',' between Tuple members.");
      }

      Tuple* xpr = newTuple(n, &parser->subXprs.data[parser->subXprs.count-n]);
      consumeXpr(parser, n);
      saveXpr(parser, xpr);
    }
    else
      consume(parser, RPAR_TOKEN, "Expect ')' after expression.");
  }
}

static void list(Parser* parser) {
  size_t n = 0;
  List* xpr = &emptyList;

  while (!parser->hadError && !match(parser, RBRACK_TOKEN)) {
    if (isAtEnd(parser))
      errorAtCurrent(parser, "Unmatched '[' to close list literal.");

    else {
      expression(parser);
      n++;
      if (!check(parser, RBRACK_TOKEN))
        consume(parser, COMMA_TOKEN, "Expect ',' between List members.");
    }
  }

  if (!parser->hadError) {
    if (n > 0) {
      xpr = newListN(n, &parser->subXprs.data[parser->subXprs.count-n]);
      consumeXpr(parser, n);
    }

    saveXpr(parser, xpr); 
  }
}

static void call(Parser* parser) {
  bool explicit = last(parser)->type == LPAR_TOKEN;
  size_t argc = arguments(parser, explicit);

  if (!parser->hadError) {
    List* args = newListN(argc, &parser->subXprs.data[parser->subXprs.count-argc]);
    consumeXpr(parser, argc);
    Tuple* xpr = newTriple(peekXpr(parser, -1), EMPTY_LIST(), TAG_OBJ(args));
    consumeXpr(parser, 1);
    saveXpr(parser, xpr);
  }
}

static size_t arguments(Parser* parser, bool explicit) {
  size_t argc = 0;

  if (explicit) {
    while(!parser->hadError &&
          !isAtEnd(parser) &&
          !check(parser, RPAR_TOKEN)) {
      expression(parser);
      argc++;
      if (!check(parser, RPAR_TOKEN))
        consume(parser, COMMA_TOKEN, "Expected ',' between arguments.");
    }

    if (!parser->hadError)
      consume(parser, RPAR_TOKEN, "Expected ')' closing list of call arguments.");

  } else {
    decOffset(parser);
    while (!parser->hadError &&
           !isAtEnd(parser) &&
           !isAtLineBreak(parser)) {
      expression(parser);
      argc++;

      if (!isAtEnd(parser) && !isAtLineBreak(parser))
        consume(parser, COMMA_TOKEN, "Expected ',' between arguments.");
    }
  }

  return argc;
}

static size_t keywords(Parser* parser, TokenType terminal) {
  size_t argc = 0;

  if (terminal != NO_TOKEN) {
    while (!parser->hadError &&
           !isAtEnd(parser) &&
           !check(parser, terminal)) {
      
    }
  }
}

static void expression(Parser* parser) {
  parsePrecedence(parser, ASSIGNMENT_PRECEDENCE);

  if (parser->panicMode)
    synchronize(parser);
}

static void parsePrecedence(Parser* parser, Precedence precedence) {
  advance(parser);
  ParseFn prefixRule = getRule(last(parser)->type)->prefix;

  if (prefixRule == NULL)
    error(parser, "Expect expression.");

  else {
    prefixRule(parser);

    while (!parser->hadError &&
           precedence <= getRule(this(parser)->type)->precedence) {
      advance(parser);
      ParseFn infixRule = getRule(last(parser)->type)->infix;

      if (infixRule == NULL)
        error(parser, "Expected expression.");
      else
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
  initValues(&parser->subXprs);
}

void freeParser(Parser* parser) {
  freeValues(&parser->subXprs);
}

Value parse(Parser* parser, Scanner* source) {
  initParser(parser, source);
  expression(parser);
  Value out = NUL_VAL;

  if (!parser->hadError) {
    out = peekXpr(parser, -1);
    consumeXpr(parser, 1);
  }

  return out;
}
