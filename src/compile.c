#include <stdarg.h>


#include "compile.h"
#include "array.h"
#include "memory.h"
#include "rlvm.h"
#include "opcodes.h"

// bytecode object implemenation
ByteCode *makeByteCode( Void )
{
  return (ByteCode*)create(&ByteCodeType);
}

Int freeByteCode( ByteCode *bytecode )
{
  freeInstr(bytecode->code);
  freeVector(bytecode->constants);
  deallocate(bytecode, sizeof(ByteCode));
  return 0;
}

Void initByteCode( ByteCode *bytecode )
{
  bytecode->code      = makeInstr(); initInstr(bytecode->code);
  bytecode->constants = makeVector(); initVector(bytecode->constants);
}

// compile implementation
static Void compileExpression( ByteCode *bytecode, Value expression );
static Void compileLiteral( ByteCode *bytecode, Value expression );
static Void emitInstr( ByteCode *bytecode, OpCode op, ... );
static Size internConstant( ByteCode *bytecode, Value expression );
static Bool isLiteral( Value value );

ByteCode *rlCompile( Value expression )
{
  ByteCode *out = makeByteCode(); initByteCode(out);

  compileExpression(out, expression);

  if ( recover() )
    {
      freeByteCode(out);
      out = NULL;
    }

  else
    emitInstr(out, OpHalt);

  return out;
}

static Void compileExpression( ByteCode *bytecode, Value expression )
{
  if ( TheVm.panic )
    return;

  if ( isLiteral(expression) )
    compileLiteral(bytecode, expression);
}

static Bool isLiteral( Value value )
{
  return !isCons(value);
}

static Void compileLiteral( ByteCode *bytecode, Value expression )
{
  Size location = internConstant(bytecode, expression);
  emitInstr(bytecode, OpConstant, location);
}

static Void emitInstr( ByteCode *bytecode, OpCode op, ... )
{
  va_list va; va_start(va, op);

  switch ( opArgc(op) )
    {
    case 0:
      apush(bytecode->code, 1, op);
      break;

    case 1:
      apush(bytecode->code, 2, op, va_arg(va, Int));
      break;

    case 2:
      apush(bytecode->code, 3, op, va_arg(va, Int), va_arg(va, Int));
      break;

    case 3:
      apush(bytecode->code, 4, op, va_arg(va, Int), va_arg(va, Int), va_arg(va, Int));
      break;

    default:
      rl_unreachable();
      break;
    }

  va_end(va);
}

static Size internConstant( ByteCode *bytecode, Value expression )
{
  Vector *constants = bytecode->constants;
  Value  *expressions = constants->data;

  for (Size i=0; i<constants->count; i++)
    {
      if (expressions[i] == expression)
	return i;
    }

  return apush(constants, 1, expression)-1;
}
