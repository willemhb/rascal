#include "reader.h"


// globals --------------------------------------------------------------------


// forward declarations -------------------------------------------------------
static bool  isAtEnd( Scanner *scanner );
static char  advance( Scanner *scanner );
static char  peek( Scanner *scanner );
static char  peekNext( Scanner *scanner );
static bool  match( Scanner *scanner, char expected );
static Token makeToken( Scanner *scanner, TokenType type );

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

static Token makeToken( Scanner *scanner, TokenType type )
{
  
}
