#ifndef rascal_compile_h
#define rascal_compile_h

#include "object.h"

/**
 *
 * Compiler and bytecode implementations go here.
 * 
 **/

// C types
typedef struct ByteCode ByteCode;
typedef struct Instr    Instr;
typedef struct Vector   Vector;

struct ByteCode
{
  OBJECT;

  Instr  *code;
  Vector *constants;
};

// forward declarations
// bytecode implemenation
ByteCode  *makeByteCode( Void );
Int        freeByteCode( ByteCode *bytecode );
Void       initByteCode( ByteCode *bytecode );

ByteCode  *rlCompile( Value expression );

// initialization
Void rlCompileInit( Void );

// globals
extern Type ByteCodeType;

#endif
