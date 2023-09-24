#include <string.h>
#include <ctype.h>

#include "util/io.h"

#include "vm.h"
#include "debug.h"
#include "scanner.h"

#define DELIMITER "(){}[]:,. \n\t\v\r"

// forward declarations for reader helpers
static Token scanNumber(Scanner* scanner);
static Token scanSymbol(Scanner* scanner);
static Token scanString(Scanner* scanner);
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

static bool isSep(int ch) {
  return strchr(DELIMITER, ch);
}

static bool isIdentifierChar(int ch) {
  return isalnum(ch) || strchr("_!?", ch);
}

static char advance(Scanner* scanner) {
  return *(scanner->current++);
}

static void skipWhiteSpace(Scanner* scanner) {
  if (isAtEnd(scanner))
    return;

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

static TokenType checkKeyword(Scanner* scanner, int start, int length,
    const char* rest, TokenType type) {
  if (scanner->current - scanner->start == start + length &&
      memcmp(scanner->start + start, rest, length) == 0) {
    return type;
  }

  return IDENTIFIER_TOKEN;
}

static bool matchChar(Scanner* scanner, char expect, bool consume) {
  if (isAtEnd(scanner))
    return false;
  
  if (*scanner->current != expect)
    return false;

  if (consume)
    scanner->current++;

  return true;
}

static bool matchStr(Scanner* scanner, char* expect, bool consume) {
  if (isAtEnd(scanner))
    return false;

  size_t n = strlen(expect);

  if (memcmp(scanner->current, expect, n))
    return false;

  if (consume)
    scanner->current +=n;

  return true;
}

#define match(s, e, c) _Generic((e), int:matchChar, char*:matchStr)(s, e, c)

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
      // atomic literals
      case '0' ... '9':
        token = scanNumber(scanner);
        break;

      case ':':
        if (match(scanner, ':', true))
          token = makeToken(scanner, COLON_COLON_TOKEN);
        else
          token = scanSymbol(scanner);
        break;
        
      case '"':
        token = scanString(scanner);
        break;

        // simple delimiters
      case '(':
        token = makeToken(scanner, LPAR_TOKEN);
        break;

      case ')':
        token = makeToken(scanner, RPAR_TOKEN);
        break;

      case '[':
        token = makeToken(scanner, LBRACK_TOKEN);
        break;

      case ']':
        token = makeToken(scanner, RBRACK_TOKEN);
        break;

      case '{':
        token = makeToken(scanner, LBRACE_TOKEN);
        break;

      case '}':
        token = makeToken(scanner, RBRACE_TOKEN);
        break;

      case ',':
        token = makeToken(scanner, COMMA_TOKEN);
        break;

      case '.':
        token = makeToken(scanner, DOT_TOKEN);
        break;

        // tricky delimiters
      case '<':
        if (match(scanner, '<', true))
          token = makeToken(scanner, LARROWS_TOKEN);
        else if (match(scanner, '=', true))
          token = makeToken(scanner, LESS_EQUAL_TOKEN);
        else
          token = makeToken(scanner, LESS_THAN_TOKEN);
        break;

      case '>':
        if (match(scanner, '>', true))
          token = makeToken(scanner, RARROWS_TOKEN);
        else if (match(scanner, '=', true))
          token = makeToken(scanner, GREATER_EQUAL_TOKEN);
        else
          token = makeToken(scanner, GREATER_THAN_TOKEN);
        break;

        // simple operators
      case '+':
        token = makeToken(scanner, PLUS_TOKEN);
        break;

      case '*':
        token = makeToken(scanner, MUL_TOKEN);
        break;

      case '/':
        token = makeToken(scanner, DIV_TOKEN);
        break;

      case '%':
        token = makeToken(scanner, REM_TOKEN);
        break;

      case '\'':
        token = makeToken(scanner, APOSTROPHE_TOKEN);
        break;

      case '|':
        token = makeToken(scanner, BAR_TOKEN);
        break;

        // tricky operators
      case '-':
        if (match(scanner, '>', true))
          token = makeToken(scanner, MATCH_TOKEN);
        else
          token = makeToken(scanner, MINUS_TOKEN);
        break;

      case '=':
        if (match(scanner, '=', true))
          token = makeToken(scanner, EQUAL_EQUAL_TOKEN);
        else
          token = makeToken(scanner, EQUAL_TOKEN);
        break;

      case '!':
        if (match(scanner, '=', true))
          token = makeToken(scanner, NOT_EQUAL_TOKEN);
        else
          token = scanIdentifier(scanner);
        break;

        // fallback
      default:
        token = scanIdentifier(scanner);
        break;
    }
  }

  writeTokens(&scanner->tokens, token);
  return token;
}

static Token scanNumber(Scanner* scanner) {
  while (!isAtEnd(scanner) && isdigit(peekChar(scanner)))
    advance(scanner);

  if (peekChar(scanner) == '.' && isdigit(peekNextChar(scanner))) {
    advance(scanner);

    while (isdigit(peekChar(scanner)))
      advance(scanner);
  }

  return makeToken(scanner, NUMBER_TOKEN);
}

static Token scanSymbol(Scanner* scanner) {
  while (!isAtEnd(scanner) && !isSep(peekChar(scanner))) {
    advance(scanner);
  }

  return makeToken(scanner, SYMBOL_TOKEN);
}

static Token scanString(Scanner* scanner) {
  while (!isAtEnd(scanner) && peekChar(scanner) != '"') {
    if (peekChar(scanner) == '\n')
      scanner->lineNo++;
    advance(scanner);
  }

  if (isAtEnd(scanner))
    return errorToken(scanner, "Unterminated string.");

  // The closing quote.
  advance(scanner);
  return makeToken(scanner, STRING_TOKEN);
}

static Token scanIdentifier(Scanner* scanner) {
  while (!isAtEnd(scanner) && isIdentifierChar(peekChar(scanner)))
    advance(scanner);

  TokenType type;

  if (match(scanner, "::", false)) // type assertion is separate infix operator
    type = IDENTIFIER_TOKEN;

  else if (match(scanner, ':', true))
    type = KEYWORD_TOKEN;

  else {
    switch (scanner->start[0]) {
      case 'a': type = checkKeyword(scanner, 1, 2, "nd", AND_TOKEN); break;
      case 'd': type = checkKeyword(scanner, 1, 1, "o", DO_TOKEN); break;
      case 'e': type = checkKeyword(scanner, 1, 2, "nd", END_TOKEN); break;
      case 'f': type = checkKeyword(scanner, 1, 4, "alse", FALSE_TOKEN); break;
      case 'n':
        switch (scanner->start[1]) {
          case 'u': type = checkKeyword(scanner, 2, 1, "l", NUL_TOKEN); break;
          case 'o': type = checkKeyword(scanner, 2, 1, "t", NOT_TOKEN); break;
          default:  type = IDENTIFIER_TOKEN; break;
        }
        break;
      case 'o': type = checkKeyword(scanner, 1, 1, "r", OR_TOKEN); break;
      case 't': type = checkKeyword(scanner, 1, 3, "rue", TRUE_TOKEN); break;
      default:  type = IDENTIFIER_TOKEN; break;
    }
  }

  return makeToken(scanner, type);
}

// generics
#include "tpl/describe.h"
ARRAY_TYPE(Tokens, Token, Token);

void initScanner(Scanner* scanner, char* source) {
  scanner->start    = source;
  scanner->current  = source;
  scanner->lineNo   = 1;
  initTokens(&scanner->tokens);
}

void freeScanner(Scanner* scanner) {
  scanner->start   = NULL;
  scanner->current = NULL;
  freeTokens(&scanner->tokens);
}

Token peekToken(Scanner* scanner, int i) {
  if (i < 0)
    i += scanner->tokens.count;

  assert(i >= 0 && i < (int)scanner->tokens.count);

  return scanner->tokens.data[i];
}

void scan(Scanner* scanner, char* source) {
  initScanner(scanner, source);

  while (!isAtEnd(scanner))
    scanToken(scanner);

  #ifdef DEBUG_SCANNER 
  displayScanner(scanner);
  #endif

}
