#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "reader.h"
#include "object.h"

typedef struct
{
  const char *start;
  const char *current;
  int length;
  int line;
} Scanner;

Scanner scanner;

void initScanner( const char *source )
{
  scanner.length = strlen(source);
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

static bool isAtEnd( void )
{
  return *scanner.current == '\0';
}

static Token makeToken( TokenType type )
{
  Token token =
    {
      .type = type,
      .start = scanner.start,
      .length = (int)(scanner.current - scanner.start),
      .line = scanner.line
    };

  return token;
}

static Token errorToken( const char *message )
{
  Token token =
    {
      .type = TOKEN_ERROR,
      .start = message,
      .length = (int)strlen(message),
      .line = scanner.line
    };
  return token;
}

static char advance( void )
{
  scanner.current++;
  return scanner.current[-1];
}

static bool match( char expected )
{
  if (isAtEnd())
    return false;

  if (*scanner.current != expected)
    return false;

  scanner.current++;

  return true;
}

static bool isIdChar( char ch, bool first )
{
  if (isdigit(ch))
    return !first;

  return ch == ':'
    ||   ch == '_'
    ||   isdigit(ch)
    ||   isalpha(ch);
}

static char peek( void )
{
  
  return *scanner.current;
}

static char peekNext( void )
{
  if (isAtEnd())
    return '\0';

  return scanner.current[1];
}

static void skipWhiteSpace( void )
{
  for (;;)
    {
      char c = peek();

      switch (c)
	{
	case '\n':
	  scanner.line++;
	  rascal_fallthrough;

	case ' ':
	case '\r':
	case '\t':
	  advance();
	  break;

	case '/':
	  if ( peekNext() == '/' )
	    {
	      // clear to end of line
	      while (peek() != '\n' && !isAtEnd() )
		advance();
	    }

	  else
	    {
	      return;
	    }
	  
	  break;
	  
	default:
	  return;
	}
    }
}

static Token string()
{
  while ( peek() != '"' && !isAtEnd() )
    {
      if ( peek() == '\n' )
	scanner.line++;
      advance();
    }

  if ( isAtEnd() )
    return errorToken( "Unterminated string." );

  // clear closing quote
  advance();

  return makeToken( TOKEN_STRING );
}

static Token number()
{
  while (isdigit(peek()))
    advance();

  if (peek() == '.' && isdigit(peekNext()))
    advance();

  while (isdigit(peek()))
    advance();

  return makeToken( TOKEN_NUMBER );
}

static TokenType checkKeyword(int start, int length,
    const char* rest, TokenType type) {
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_SYMBOL;
}

static TokenType identifierType()
{
  
  switch (scanner.start[0])
    {
      // special case
    case ':': return TOKEN_KEYWORD;
      
      // single branching
    case 'a': return checkKeyword( 1, 2, "nd", TOKEN_AND );
    case 'i': return checkKeyword( 1, 1, "f", TOKEN_IF);
    case 'n': return checkKeyword( 1, 2, "il", TOKEN_NIL);
    case 'o': return checkKeyword( 1, 1, "r", TOKEN_OR);
    case 'p': return checkKeyword( 1, 4, "rint", TOKEN_PRINT);
    case 'd': return checkKeyword( 1, 1, "o", TOKEN_DO );
    case 'm': return checkKeyword( 1, 2, "ac", TOKEN_MAC );
    case 'v': return checkKeyword( 1, 2, "al", TOKEN_VAL );
    case 'q': return checkKeyword( 1, 4, "uote", TOKEN_QUOTE );
    case 'w': return checkKeyword( 1, 3, "ith", TOKEN_WITH );

      // double branching
    case 't':
      if (scanner.current - scanner.start > 1)
	{
	  switch (scanner.start[1])
	    {
	    case 'y': return checkKeyword( 2, 2, "pe", TOKEN_TYPE );
	    case 'r': return checkKeyword( 2, 2, "ue", TOKEN_TRUE );
	    }
	}
      break;
      
    case 'f':
      if (scanner.current - scanner.start > 1)
	{
	  switch (scanner.start[1])
	    {
	    case 'a': return checkKeyword( 2, 3, "lse", TOKEN_FALSE );
	    case 'u': return checkKeyword( 2, 1, "n", TOKEN_FUN );
	    }
	}
      break;
      
    case 'e':
      if (scanner.current - scanner.start > 1)
	{
	  switch (scanner.start[1])
	    {
	    case 'n': return checkKeyword( 2, 1, "d", TOKEN_END );
	    case 'l': return checkKeyword( 2, 2, "se", TOKEN_ELSE );
	    }
	}
      break;
    }

  return TOKEN_SYMBOL;
}

static Token identifier()
{
  advance();
  
  while ( isIdChar(peek(), false) )
    advance();

  return makeToken( identifierType() );
}

Token scanToken( void )
{
  skipWhiteSpace();
  scanner.start = scanner.current;

  if (isAtEnd())
    return makeToken( TOKEN_EOF );

  char c = advance();
  
  if (isdigit(c))
    return number();

  if ( isIdChar(c, true) )
    return identifier();

    switch (c)
      {
      case '(': return makeToken(TOKEN_LEFT_PAREN);
      case ')': return makeToken(TOKEN_RIGHT_PAREN);
      case '{': return makeToken(TOKEN_LEFT_BRACE);
      case '}': return makeToken(TOKEN_RIGHT_BRACE);
      case '[': return makeToken(TOKEN_LEFT_BRACK);
      case ']': return makeToken(TOKEN_RIGHT_BRACK);
      case ';': return makeToken(TOKEN_SEMICOLON);
      case ',': return makeToken(TOKEN_COMMA);
      case '.': return makeToken(TOKEN_DOT);
      case '+': return makeToken(TOKEN_PLUS);
      case '!': return makeToken(TOKEN_BANG);
      case '*': return makeToken(TOKEN_STAR);
      case '/':
	return makeToken( match('=') ? TOKEN_SLASH_EQUAL : TOKEN_BANG );

      case '=':
	return makeToken( match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL );

      case '-':
	if ( match( '>' ) )
	  return makeToken( TOKEN_LEFT_ARROW );

	return makeToken( TOKEN_MINUS );

      case '<':
	if ( match('=') )
	  return makeToken( TOKEN_LESS_EQUAL );

	if ( match('-') )
	  return makeToken( TOKEN_LEFT_ARROW );

	return makeToken( TOKEN_LESS );

      case '>':
	return makeToken( match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER );
      case '"':
	return string();
      }

  return errorToken( "Unexpected character." );
}
