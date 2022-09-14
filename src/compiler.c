#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "reader.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif


typedef struct
{
  Token current;
  Token previous;
  bool  hadError;
  bool  panicMode;
} Parser;

typedef enum
  {
    PREC_NONE,
    PREC_ASSIGNMENT, // <-
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // =, /=
    PREC_COMPARISON, // <, >, <=, >=
    PREC_TERM,       // +, -
    PREC_FACTOR,     // *, /
    PREC_UNARY,      // 1, -
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

Parser parser;
Chunk *compilingChunk;

// forward declarations -------------------------------------------------------
static Chunk     *currentChunk( void );
static void       errorAtCurrent( const char *message );
static void       errorAt( Token *token, const char *message );
static void       error( const char *message );
static void       advance( void );
static void       consume( TokenType type, const char *message );
static void       emitByte( uint16_t byte );
static void       emitBytes( uint16_t byte1, uint16_t byte2 );
static void       emitReturn( void );
static void       endCompiler( void );
static void       binary( void );
static void       literal( void );
static void       expression( void );
static ParseRule *getRule( TokenType type );
static void       grouping( void );
static uint16_t   makeConstant( Value value );
static void       emitConstant( Value value );
static void       number( void );
static void       unary( void );
static void       parsePrecedence( Precedence precedence );



// implementations ------------------------------------------------------------
static Chunk *currentChunk( void )
{
  return compilingChunk;
}

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
    case TOKEN_EQUAL_EQUAL:   emitByte(OP_SAME); break;
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
    [TOKEN_SEMICOLON]     = { NULL,     NULL,   PREC_NONE       },
    [TOKEN_SLASH]         = { NULL,     binary, PREC_FACTOR     },
    [TOKEN_STAR]          = { NULL,     binary, PREC_FACTOR     },
    
    [TOKEN_BANG]          = { unary,    NULL,   PREC_NONE       },
    [TOKEN_BANG_EQUAL]    = { NULL,     binary, PREC_EQUALITY   },
    [TOKEN_EQUAL]         = { NULL,     binary, PREC_EQUALITY   },
    [TOKEN_EQUAL_EQUAL]   = { NULL,     binary, PREC_EQUALITY   },
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
