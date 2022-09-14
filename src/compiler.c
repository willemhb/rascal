#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "reader.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

Chunk *compilingChunk;

// forward declarations -------------------------------------------------------
static Chunk     *currentChunk( void );
static void       advance( void );
static void       consume( TokenType type, const char *message );
static void       emitByte( uint16_t byte );
static void       emitBytes( uint16_t byte1, uint16_t byte2 );
static void       emitReturn( void );
static void       endCompiler( void );
static uint16_t   makeConstant( Value value );
static void       emitConstant( Value value );


// implementations ------------------------------------------------------------
static Chunk *currentChunk( void )
{
  return compilingChunk;
}

static void advance( void )
{
  parser.previous = parser.current;

  for (;;)
    {
      parser.current = scanToken();
      if ( parser.current.type != TOKEN_ERROR )
	break;

      errorAtCurrent( parser.current.start );
    }
}

static void consume(TokenType type, const char *message)
{
  if (parser.current.type == type)
    {
      advance();
      return;
    }

  errorAtCurrent(message);
}

static void emitByte( uint16_t byte )
{
  writeChunk( currentChunk(), byte, parser.previous.line );
}

static void emitBytes( uint16_t byte1, uint16_t byte2 )
{
  emitByte( byte1 );
  emitByte( byte2 );
}

static void emitReturn( void )
{
  emitByte( OP_RETURN );
}

static void endCompiler ( void )
{
  emitReturn();
}

static void binary( void )
{
  TokenType operatorType = parser.previous.type;
  ParseRule *rule = getRule( operatorType );
  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (operatorType)
    {
    case TOKEN_SLASH_EQUAL:   emitBytes(OP_EQUAL, OP_NOT); break;
    case TOKEN_EQUAL:         emitByte(OP_EQUAL); break;
    case TOKEN_GREATER:       emitByte(OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
    case TOKEN_LESS:          emitByte(OP_LESS); break;
    case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
    case TOKEN_PLUS:          emitByte(OP_ADD); break;
    case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
    case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
    case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
    default: return;
    }
}

static void literal( void )
{
  switch (parser.previous.type)
    {
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_NIL:   emitByte(OP_NIL); break;
    case TOKEN_TRUE:  emitByte(OP_TRUE); break;
    }
}

static void expression( void )
{
  parsePrecedence( PREC_ASSIGNMENT );
}

static void grouping( void )
{
  expression();
  consume( TOKEN_RIGHT_PAREN, "Expect ')', after expression." );
}

static uint16_t makeConstant( Value value )
{
  int constant = addConstant( currentChunk(), value );

  if ( constant > UINT16_MAX )
    {
      error( "Too many constants in one chunk." );
      return 0;
    }

  return (uint16_t)constant;
}

static void emitConstant( Value value )
{
  emitBytes( OP_CONSTANT, makeConstant( value ) );
}

static void number ( void )
{
  double value = strtod( parser.previous.start, NULL );
  emitConstant( value );
}

static void unary ( void )
{
  TokenType operatorType = parser.previous.type;

  parsePrecedence( PREC_UNARY );

  switch (operatorType)
    {
    case TOKEN_BANG:
      emitByte(OP_NOT); break;

    case TOKEN_MINUS:
      emitByte(OP_NEGATE); break;

    default: return;
    }
}

bool compile( const char *source, Chunk *chunk )
{
  initScanner( source );
  compilingChunk = chunk;

  parser.hadError  = false;
  parser.panicMode = false;
  
  advance();
  expression();
  consume(TOKEN_EOF, "Expect end of expression." );
  endCompiler();
  return !parser.hadError;
}
