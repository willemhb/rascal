#ifndef rascal_compiler_h
#define rascal_compiler_h

#include "object.h"

// C types --------------------------------------------------------------------
typedef enum
  {
   // stack manipulation ------------------------------------------------------
   OP_PUSH,
   OP_POP,

   // load & store instructions -----------------------------------------------
   OP_CONSTANT,
   OP_LOAD_GLOBAL,
   OP_STORE_GLOBAL,
   OP_LOAD_LOCAL,
   OP_STORE_LOCAL,
   OP_LOAD_UPVALUE,
   OP_STORE_UPVALUE,

   // control flow -----------------------------------------------------------
   OP_JUMP,
   OP_JUMP_TRUE,
   OP_JUMP_FALSE,
   OP_CALL,
   OP_RETURN,

   // variables/closures ------------------------------------------------------
   OP_CLOSURE,
   OP_CAPTURE_UPVALUE,
   OP_CLOSE_UPVALUE,
  } OpCode;

struct ByteCode
{
  OBJ_HEAD(UInt16);

  CodeBuffer   *code;
  ArrayList    *constants;
};

// forward declarations -------------------------------------------------------
// ByteCode object api --------------------------------------------------------
ByteCode *newByteCode( void );

// compiler API ---------------------------------------------------------------
ByteCode *compile( Obj *source );



#define compileFile(source)				\
  _Generic((source),					\
	   String*:compileFileFromString,		\
	   Char*:compileFileFromCstring)(source)

Void compileFileFromString( String *src );
Void compileFileFromCstring( Char *src );

#define asByteCode(val)  (asObjType(ByteCode, val))
#define bcCode(val)      (asByteCode(val)->code)
#define bcConstants(val) (asByteCode(val)->constants)

#endif
