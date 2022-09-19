#include <stdarg.h>

#include "compiler.h"
#include "function.h"
#include "binary.h"
#include "memory.h"

// C types --------------------------------------------------------------------

typedef enum
  {
    FN_FUNCTION,
    FN_MACRO,
    FN_SCRIPT,
  } FnType;

typedef struct Compiler
{
  // enclosing context --------------------------------------------------------
  struct Compiler *enclosing;

  // being compiled -----------------------------------------------------------
  List   *source;
  Value   expression;

  // targets ------------------------------------------------------------------
  CodeBuffer *code;
  ArrayList  *constants;

  // environment information --------------------------------------------------
  Vector *locals;
  Vector *upvalues;
  Map    *globals;
  Map    *syntax;
} Compiler;

// forward declarations -------------------------------------------------------
// compiler helpers -----------------------------------------------------------
static Void  emitInstruction( Compiler *compiler, OpCode instr, ...);
static Arity addConstant( Compiler *compiler, Value val );
static Void  resolveName( Compiler *compiler, Atom *name );

// Compiler methods -----------------------------------------------------------
#define MIN_INSTRUCTION_COUNT 64

Compiler *newCompiler( void )
{
  return allocate( sizeof(Compiler) );
}

Void initCompiler( Compiler *compiler, Compiler *enclosing )
{
  compiler->enclosing = enclosing;
  compiler->locals     = NULL;
  compiler->upvalues   = NULL;
  compiler->globals    = NULL;
  compiler->syntax     = NULL;

  compiler->code       = allocate( VAL_CODEBUFFER, MIN_INSTRUCTION_COUNT );
}



// utilities ------------------------------------------------------------------
static Arity opArgc( OpCode instr )
{
  switch (instr)
    {
    case OP_PUSH:
    case OP_POP:
    case OP_RETURN:
      return 0;

    case OP_CONSTANT ... OP_CALL:
    case OP_CLOSURE:
    case OP_CAPTURE_UPVALUE:
    case OP_CLOSE_UPVALUE:
      return 1;
    }
}

static Void emitInstruction( Compiler* compiler, OpCode instr, ...)
{
  va_list va;
  va_start(va, instr);

  va_end(va);
}
