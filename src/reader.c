#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "reader.h"
#include "object.h"


// globals --------------------------------------------------------------------
Scanner scanner;
Parser parser;

// C types --------------------------------------------------------------------
/*
  supported operators (besides standard arithmetic operators)

  xs           <- [1, 2, 3]
  ys           <- [3, 2, 1]
  x            <- :an_atom

  1..10        -> range(1, 10, 1)
  1,3..10      -> range(1, 10, 2)
  x >> [1, 2]  -> prepend(x, [1, 2])
  x << [1, 2]  -> append(x, [1, 2])
  x :: [1, 2]  -> conj(x, [1, 2])
  [x] + [1, 2] -> cat([x], [1, 2])

  xs[1]        -> index(xs, 1)
  xs[1:2]      -> index(xs, slice(1, 2, 1))
  xs[(<3)]     -> filter((x) -> x < 3, xs)
  xs[:(<10)]   -> takewhile((x) -> x < 10, xs)
  xs[(<10):]   -> dropwhile((x) -> x < 10, xs)
  xs .(<10)    -> map((x) -> x < 10, xs)
  xs .<= ys    -> map(<=, x, y)
  xs .<= 2     -> map((x) -> x <= 2, xs)


*/
typedef enum
  {
    PREC_NONE,
    PREC_ASSIGNMENT, // <-, ->
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_NOT,        // not
    PREC_EQUALITY,   // =, /=
    PREC_COMPARISON, // <, >, <=, >=
    PREC_TERM,       // +, -
    PREC_FACTOR,     // *, /
    PREC_UNARY,      // +, -, ~
    PREC_CALL,       // ., ()
    PREC_PRIMARY
  } Precedence;

typedef void (*ParseFn)(void);

typedef struct
{
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

// forward declarations -------------------------------------------------------
static void       errorAtCurrent( const char *message );
static void       errorAt( Token *token, const char *message );
static void       error( const char *message );

static void       binary( void );
static void       literal( void );
static void       expression( void );
static void       grouping( void );
static void       number( void );
static void       unary( void );
static void       string( void );
static void       symbol( void );
static void       vector( void );
static void       list( void );

static ParseRule *getRule( TokenType type );
static void       parsePrecedence( Precedence precedence );


// parse rules ----------------------------------------------------------------
ParseRule rules[] =
  {
    [TOKEN_LEFT_PAREN]    = { grouping, NULL,   PREC_NONE       },
    [TOKEN_RIGHT_PAREN]   = { NULL,     NULL,   PREC_NONE       },
    [TOKEN_LEFT_BRACE]    = { NULL,     NULL,   PREC_NONE       }, 
    [TOKEN_RIGHT_BRACE]   = { NULL,     NULL,   PREC_NONE       },
    [TOKEN_LEFT_BRACK]    = { NULL,     NULL,   PREC_NONE       },
    [TOKEN_RIGHT_BRACK]   = { NULL,     NULL,   PREC_NONE       },
    
    [TOKEN_COMMA]         = { NULL,     NULL,   PREC_NONE       },
    [TOKEN_DOT]           = { NULL,     NULL,   PREC_NONE       },
    [TOKEN_MINUS]         = { unary,    binary, PREC_TERM       },
    [TOKEN_PLUS]          = { NULL,     binary, PREC_TERM       },
    [TOKEN_SLASH]         = { NULL,     binary, PREC_FACTOR     },
    [TOKEN_STAR]          = { NULL,     binary, PREC_FACTOR     },
    
    [TOKEN_EQUAL]         = { NULL,     binary, PREC_EQUALITY   },
    [TOKEN_GREATER]       = { NULL,     binary, PREC_COMPARISON },
    [TOKEN_GREATER_EQUAL] = { NULL,     binary, PREC_COMPARISON },
    [TOKEN_LESS]          = { NULL,     binary, PREC_COMPARISON },
    [TOKEN_LESS_EQUAL]    = { NULL,     binary, PREC_COMPARISON },
    [TOKEN_LEFT_ARROW]    = { NULL,     NULL,   PREC_ASSIGNMENT },
    [TOKEN_RIGHT_ARROW]   = { NULL,     NULL,   PREC_ASSIGNMENT },
  
    [TOKEN_SYMBOL]        = { NULL,     NULL,   PREC_NONE       },
    [TOKEN_KEYWORD]       = { NULL,     NULL,   PREC_NONE       },
    [TOKEN_STRING]        = { string,   NULL,   PREC_NONE       },
    [TOKEN_NUMBER]        = { number,   NULL,   PREC_NONE       },
    
    [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_TYPE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
    [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
    [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DO]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_MAC]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_END]           = {NULL,     NULL,   PREC_NONE},  
    [TOKEN_WITH]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
    [TOKEN_VAL]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_QUOTE]         = {NULL,     NULL,   PREC_NONE},
    
    [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
  };

// error helpers --------------------------------------------------------------
static void errorAtCurrent( const char *message )
{
  errorAt( &parser.previous, message );
}

static void errorAt( Token *token, const char *message )
{
  if (parser.panicMode)
    return;
  
  parser.panicMode = true;
  
  fprintf( stderr, "[line %d] Error", token->line );
  
  if ( token->type == TOKEN_EOF )
    {
      fprintf( stderr, "at end" );
    }

  else if ( token->type == TOKEN_ERROR )
    {
      
    }

  else
    {
      fprintf( stderr, "at '%.*s'", token->length, token->start );
    }

  fprintf( stderr, ": %s\n", message );
  parser.hadError = true;
}

static void error( const char *message )
{
  errorAt( &parser.previous, message );
}

// parser implementation ------------------------------------------------------

static void parsePrecedence( Precedence precedence )
{
  advance();

  ParseFn prefixRule = getRule(parser.previous.type)->prefix;

  if (prefixRule == NULL)
    {
      error( "Expect expression." );
      return;
    }

  prefixRule();

  while (precedence <= getRule(parser.current.type)->precedence)
    {
      advance();
      ParseFn infixRule = getRule(parser.previous.type)->infix;
      infixRule();
    }
}

static ParseRule *getRule(TokenType type)
{
  return &rules[type];
}


// scanner implementation -----------------------------------------------------
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
    case 'n': return checkKeyword( 1, 2, "il", TOKEN_NIL );
    case 't': return checkKeyword( 1, 3, "rue", TOKEN_TRUE );
    case 'f': return checkKeyword( 1, 4, "alse", TOKEN_FALSE );
    case 'e': return checkKeyword( 1, 2, "nd", TOKEN_END );
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
      case ',': return makeToken(TOKEN_COMMA);
      case '.': return makeToken(TOKEN_DOT);
      case '+': return makeToken(TOKEN_PLUS);
      case '*': return makeToken(TOKEN_STAR);
      case '/':
	return makeToken( match('=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH );

      case '=':
	return makeToken( TOKEN_EQUAL );

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

Value readExpression( void )
{
  
}
