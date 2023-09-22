#include <string.h>
#include <ctype.h>

#include "util/io.h"

#include "vm.h"
#include "lexer.h"

#define SYMPUNCT "?!_-$%&@~`'\"\\/"

// forward declarations for reader helpers
static Token scanNumber(Scanner* scanner);
static Token scanSymbol(Scanner* scanner);
static Token scanIdentifier(Scanner* scanner);

// static helpers
static bool isAtEnd(Scanner* scanner) {
  return *scanner->current == '\0';
}

static char peekChar(Scanner* scanner) {
  return *scanner->current;
}

static char peekNextChar(Scanner* scanner) {
  if (isAtEnd(scanner))
    return '\0';

  return scanner->current[1];
}

static bool isSymbolChar(int ch) {
  return isalnum(ch) || strchr(SYMPUNCT, ch);
}

static char advance(Scanner* scanner) {
  return *(scanner->current++);
}

static void skipWhiteSpace(Scanner* scanner) {
  for (;;) {
    char c = peekChar(scanner);

    switch (c) {
      /* common whitespace */
      case ' ':
      case '\r':
      case '\t':
      case '\v':
        advance(scanner);
        break;
        
        /* newline */
      case '\n':
        scanner->lineNo++;
        advance(scanner);
        break;
        
        /* line comment */
      case '#':
        while (peekChar(scanner) != '\n' && !isAtEnd(scanner)) {
          advance(scanner);
        }
        break;

      default:
        return;
    }
  }
}

static Token makeToken(Scanner* scanner, TokenType type) {
  Token token;

  token.start  = scanner->start;
  token.length = scanner->current - scanner->start;
  token.type   = type;
  token.lineNo = scanner->lineNo;

  return token;
}

static Token errorToken(Scanner* scanner, const char* message) {
  Token token;

  token.start  = message;
  token.length = strlen(message);
  token.type   = ERROR_TOKEN;
  token.lineNo = scanner->lineNo;

  scanner->hadError = true;

  return token;
}

static Token scanToken(Scanner* scanner) {
  // read next token from source
  skipWhiteSpace(scanner);

  scanner->start = scanner->current;

  Token token;

  if (isAtEnd(scanner)) {
    token = makeToken(scanner, EOF_TOKEN);
  } else {
    char c = advance(scanner);

    switch (c) {
      case '0' ... '9': token = scanNumber(scanner);             break;
      case ':':         token = scanSymbol(scanner);             break;
      case '+':         token = makeToken(scanner, PLUS_TOKEN);  break;
      case '-':         token = makeToken(scanner, MINUS_TOKEN); break;
      case '*':         token = makeToken(scanner, MUL_TOKEN);   break;
      case '/':         token = makeToken(scanner, DIV_TOKEN);   break;
      case '^':         token = makeToken(scanner, EXP_TOKEN);   break;
      case '(':         token = makeToken(scanner, LPAR_TOKEN);  break;
      case ')':         token = makeToken(scanner, RPAR_TOKEN);  break;
      case ',':         token = makeToken(scanner, COMMA_TOKEN); break;
      default:          token = scanIdentifier(scanner);         break;
    }
  }

  writeTokens(&scanner->tokens, token);
  return token;
}

static Token scanNumber(Scanner* scanner) {
  while (isdigit(peekChar(scanner)))
    advance(scanner);

  if (peekChar(scanner) == '.' && isdigit(peekNextChar(scanner))) {
    advance(scanner);

    while (isdigit(peekChar(scanner)))
      advance(scanner);
  }

  return makeToken(scanner, NUMBER_TOKEN);
}

static Token scanSymbol(Scanner* scanner) {
  size_t length = 0;
  
  while (isSymbolChar(peekChar(scanner))) {
    length++;
    advance(scanner);
  }

  if (length == 0)
    return errorToken(scanner, "Illegal empty symbol token.\n");

  scanner->start++; // clear the initial ':'
  return makeToken(scanner, SYMBOL_TOKEN);
}

static Token scanIdentifier(Scanner* scanner) {
  while (isSymbolChar(peekChar(scanner)))
    advance(scanner);

  return makeToken(scanner, IDENTIFIER_TOKEN);
}

// generics
#include "tpl/describe.h"

ARRAY_TYPE(Tokens, Token);

void initScanner(Scanner* scanner, char* source) {
  scanner->start    = source;
  scanner->current  = source;
  scanner->lineNo   = 1;
  scanner->hadError = false;
  initTokens(&scanner->tokens);
}

void freeScanner(Scanner* scanner) {
  freeTokens(&scanner->tokens);
}

Token peekToken(Scanner* scanner, int i) {
  if (i < 0)
    i += scanner->tokens.count;

  assert(i >= 0 && i < (int)scanner->tokens.count);

  return scanner->tokens.data[i];
}

bool lexInput(char* source) {
  Scanner* scanner = &vm.scanner;

  initScanner(scanner, source);

  while (!isAtEnd(scanner) && !scanner->hadError)
    scanToken(scanner);

  return scanner->hadError;
}
