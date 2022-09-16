#include <string.h>

#include "reader.h"
#include "atom.h"

// globals --------------------------------------------------------------------

// forward declarations -------------------------------------------------------
static bool      isAtEnd( Scanner *scanner );
static char      advance( Scanner *scanner );
static char      peek( Scanner *scanner );
static char      peekNext( Scanner *scanner );
static bool      match( Scanner *scanner, char expected );
static Token     makeToken( Scanner *scanner, TokenType type );
static Token     errorToken( Scanner *scanner, const char *message );
static void      skipWhiteSpace( Scanner *scanner );
static TokenType checkKeyword( int start, int length,
			       const char *rest, TokenType type);
static TokenType identifierType( );

// implementations ------------------------------------------------------------
void initScanner( Scanner *scanner, const char *source )
{
  scanner->start   = source;
  scanner->current = source;
  scanner->line    = 1;
}

static bool isAtEnd( Scanner *scanner )
{
  return *scanner->current == '\0';
}

static char advance( Scanner *scanner )
{
  scanner->current++;
  return scanner->current[-1];
}

static char peek( Scanner *scanner )
{
  return *scanner->current;
}

static char peekNext( Scanner *scanner )
{
  if ( isAtEnd( scanner ) )
    return '\0';

  return scanner->current[1];
}

 static bool match( Scanner *scanner, char expected )
{
  if ( isAtEnd( scanner ) )
    return false;

  if (*scanner->current != expected)
    return false;

  scanner->current++;
  return true;
}

static Token makeToken(Scanner *scanner, TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner->start;
  token.length = (int)(scanner->current - scanner->start);
  token.line = scanner->line;
  return token;
}
//< make-token
//> error-token
static Token errorToken(Scanner *scanner, const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner->line;
  return token;
}

static void skipWhitespace( Scanner *scanner ) {
  for (;;) {
    char c = peek( scanner );
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advance( scanner );
        break;
//> newline
      case '\n':
        scanner->line++;
        advance( scanner );
        break;
//< newline
      case '/':
        if (peekNext( scanner ) == '/') {
          // A comment goes until the end of the line.
          while (peek( scanner ) != '\n' && !isAtEnd( scanner ))
	    advance( scanner );
        } else {
          return;
        }
        break;
      default:
        return;
    }
  }
}

static TokenType checkKeyword(int start, int length,
    const char* rest, TokenType type) {
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_SYMBOL;
}

static TokenType identifierType( Scanner *scanner ) {
  switch (scanner->start[0]) {
  case ':':   return TOKEN_ATOM;
  case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
  case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
  case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);

  case 'f':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
          case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
          case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
        }
      }
      break;
//< keyword-f
    case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
    case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
//> keyword-t
    case 't':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
          case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
        }
      }
      break;
//< keyword-t
    case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }

//< keywords
  return TOKEN_IDENTIFIER;
}
//< identifier-type
//> identifier
static Token identifier() {
  while (isAlpha(peek()) || isDigit(peek())) advance();
  return makeToken(identifierType());
}
//< identifier
//> number
static Token number() {
  while (isDigit(peek())) advance();

  // Look for a fractional part.
  if (peek() == '.' && isDigit(peekNext())) {
    // Consume the ".".
    advance();

    while (isDigit(peek())) advance();
  }

  return makeToken(TOKEN_NUMBER);
}
//< number
//> string
static Token string( Scanner *scanner ) {
  while (peek( scanner ) != '"' && !isAtEnd( scanner )) {
    if (peek( scanner ) == '\n') scanner->line++;
    advance( scanner );
  }

  if (isAtEnd( scanner )) return errorToken(scanner, "Unterminated string.");

  // The closing quote.
  advance( scanner );
  return makeToken(scanner, TOKEN_STRING);
}

Token scanToken( Scanner *scanner )
{
  skipWhitespace( scanner );
  scanner->start = scanner->current;

  if (isAtEnd( scanner ))
    return makeToken(scanner, TOKEN_EOF);
  
  char c = advance( scanner );

  if (isAlpha(c))
    return identifier();
  
  if (isDigit(c))
    return number();
  
  switch (c) {
  case '(': return makeToken(TOKEN_LEFT_PAREN);
  case ')': return makeToken(TOKEN_RIGHT_PAREN);
  case '{': return makeToken(TOKEN_LEFT_BRACE);
  case '}': return makeToken(TOKEN_RIGHT_BRACE);
  case '[': return makeToken(TOKEN_LEFT_BRACK);
  case ']': return makeToken(TOKEN_RIGHT_BRACK);
  case ',': return makeToken(TOKEN_COMMA);
  case '.': return makeToken(TOKEN_DOT);
  case '-': return makeToken(TOKEN_MINUS);
  case '+': return makeToken(TOKEN_PLUS);
  case '/': return makeToken(TOKEN_SLASH);
  case '*': return makeToken(TOKEN_STAR);

    // one- or two-character tokens.
  case '-':
    return makeToken( match('>')
		      ? TOKEN_RIGHT_ARROW
		      : TOKEN_MINUS );
  case '%':
    return makeToken( match('{')
		      ? TOKEN_PERCENT_LEFT_BRACE
		      : TOKEN_PERCENT );
    case '=':
      return makeToken(
          match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
      return makeToken(
          match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(
          match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
//< two-char
//> scan-string
    case '"': return string();
//< scan-string
  }
//< scan-char

  return errorToken( scanner, "Unexpected character." );
}
