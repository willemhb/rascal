#include <stdlib.h>
#include <string.h>

/*
#include "value.h"
#include "object.h"
#include "scanner.h"
#include "parser.h"

// internal API
// grammar rules (this is a recursive descent parser, so each production has a procedure)
static Value parseExpression(Parser* parser);
static Value parseAddition(Parser* parser);
static Value parseCall(Parser* parser);
static Value parseMultiplication(Parser* parser);
static Value parseExponentiation(Parser* parser);
static Value parseBasic(Parser* parser);
static Value parseError(Parser* parser, const char* message);

// misc helpers
static Value consume(Parser* parser, TokenType expect, const char* message);

static Token advance(Parser* parser) {
  // don't advance past end of input
  if (parser->offset == 0 || parser->current.type != EOF_TOKEN) {
    if (parser->offset > 0)
      parser->previous = parser->current;

    parser->current = peekToken(parser->scanner, parser->offset++);
  }

  return parser->current;
}

static Token peekNext(Parser* parser) {
  if (parser->current.type == EOF_TOKEN)
    return parser->current;

  return peekToken(parser->scanner, parser->offset+1);
}

static Value parseBasic(Parser* parser) {
  Token token = parser->current;
  Value out;

  if (token.type == LPAR_TOKEN) {
    advance(parser); // clear left parenthesis
    out = parseExpression(parser);
    consume(parser, RPAR_TOKEN, "unmatched '('.");
  } else {
    // copy token to properly null-terminated buffer
    char buffer[token.length+1];
    memcpy(buffer, token.start, token.length);
    buffer[token.length] = '\0';

    if (token.type == NUMBER_TOKEN) {
      char* bufEnd;
      Number val = strtod(buffer, &bufEnd);
      assert(*bufEnd == '\0');
      out = TAG_NUM(val);
    } else if (token.type == SYMBOL_TOKEN) {
      Symbol* val = getSymbol(buffer);
      out = TAG_OBJ(val);
    } else if (token.type == IDENTIFIER_TOKEN) {
      Symbol* val = getSymbol(buffer);
      Tuple*  astNode = newTriple(TAG_OBJ(val), TAG_OBJ(&emptyList), NIL_VAL);
      out = TAG_OBJ(astNode);
    } else {
      out = parseError(parser, "Unreadable token");
    }
  }

  return out;
}

// external API
void initParser(Parser* parser, Scanner* scanner) {
  parser->offset     = 0;
  parser->hadError   = false;
  parser->scanner    = scanner;
  parser->expression = NIL_VAL;
}

bool parseInput(Parser* parser) {
  
}

*/
