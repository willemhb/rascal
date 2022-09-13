#include <stdio.h>
#include <stdarg.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

VM vm;

// forward declarations -------------------------------------------------------
static void resetStack( void );
static void runtimeError(const char *format, ...);
static Value peek( int distance );
static bool isFalsey( Value x );
static InterpretResult run( void );


// implementations ------------------------------------------------------------
static void resetStack( void )
{
  vm.stackTop = vm.stack;
}

static void runtimeError(const char *format, ...)
{
  va_list(args);
  va_start(args, format);
  vfprintf( stderr, format, args );
  va_end(args);
  fprintf( stderr, "\n" );

  size_t instruction = vm.ip - vm.chunk->code - 1;
  int line = vm.chunk->lines[instruction];
  fprintf( stderr, "[line %d] in script\n", line );
  resetStack();
}

void initVM( void )
{
  resetStack();
}

void freeVM( void )
{
  
}



void push( Value value )
{
  *vm.stackTop = value;
   vm.stackTop++;
}

Value pop( void )
{
  vm.stackTop--;
  return *vm.stackTop;
}

static Value peek( int distance )
{
  return vm.stackTop[-1 - distance];
}

static bool isFalsey( Value x )
{
  return x == NIL_VAL || x == FALSE_VAL;
}

static InterpretResult run( void )
{
#define READ_BYTE()     (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op)				\
  if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))		\
    {								\
      runtimeError("Operands must be numbers.");		\
      return INTERPRET_RUNTIME_ERROR;				\
    }								\
  tmpy = pop();							\
  tmpx = pop();							\
  push( valueType( AS_NUMBER(tmpx) op AS_NUMBER(tmpy)  ) );	\
  goto lbl_dispatch
  
  static void *labels[] =
    {
      // loads ----------------------------------------------------------------
      [OP_CONSTANT] = &&lbl_constant,

      [OP_NIL]      = &&lbl_nil,      [OP_TRUE]     = &&lbl_true,
      [OP_FALSE]    = &&lbl_false,
	
      // unary logical --------------------------------------------------------
      [OP_NOT]      = &&lbl_not,

	// binary logical -----------------------------------------------------
	[OP_EQUAL] = &&lbl_equal,      [OP_SAME] = &&lbl_same,

      // unary arithmetic -----------------------------------------------------
      [OP_NEGATE]   = &&lbl_negate,

      // binary arithmetic ----------------------------------------------------
      [OP_ADD]      = &&lbl_add,      [OP_SUBTRACT] = &&lbl_subtract,
      [OP_MULTIPLY] = &&lbl_multiply, [OP_DIVIDE]   = &&lbl_divide,

      // stores ---------------------------------------------------------------
      [OP_RETURN]   = &&lbl_return
    };

  uint16_t op;

  Value tmpx, tmpy;
  
 lbl_dispatch:
#ifdef DEBUG_TRACE_EXECUTION
  showStack();
  disassembleInstruction( vm.chunk,
			  (int)(vm.ip - vm.chunk->code) );
#endif
  
  op = READ_BYTE();
  goto *labels[op];

 lbl_constant:
  tmpx = READ_CONSTANT();
  push(tmpx);
  goto lbl_dispatch;

 lbl_nil:
  push(NIL_VAL);
  goto lbl_dispatch;

 lbl_true:
  push(TRUE_VAL);
  goto lbl_dispatch;

 lbl_false:
  push(FALSE_VAL);
  goto lbl_dispatch;

 lbl_not:
  tmpx = pop();
  tmpy = BOOL_VAL(isFalsey(tmpx));
  push(tmpy);
  goto lbl_dispatch;

 lbl_negate:
  if (!IS_NUMBER(peek(0)))
    {
      runtimeError( "Operand must be a number." );
      return INTERPRET_RUNTIME_ERROR;
    }
  
  tmpx = pop();
  tmpy = NUMBER_VAL(-AS_NUMBER(tmpx));
  push(tmpy);
  goto lbl_dispatch;

 lbl_add:
  BINARY_OP(NUMBER_VAL, +);

 lbl_subtract:
  BINARY_OP(NUMBER_VAL, -);

 lbl_multiply:
  BINARY_OP(NUMBER_VAL, *);

 lbl_divide:
  BINARY_OP(NUMBER_VAL, /);

 lbl_return:
  return INTERPRET_OK;

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret( const char *source )
{
  Chunk chunk;
  initChunk(&chunk);
  
  if ( !compile( source, &chunk ) )
    {
      freeChunk( &chunk );
      return INTERPRET_COMPILE_ERROR;
    }

  vm.chunk = &chunk;
  vm.ip    = vm.chunk->code;

  InterpretResult result = run();
  freeChunk( &chunk );
  return result;
}
