#ifndef rascal_opcode_h
#define rascal_opcode_h

#include "value.h"

typedef UShort OpCode;
typedef enum OpCodes OpCodes;

enum OpCodes
  {
    // misc
    OpNothing,   // noop

    // load/store
    OpLoadConst,  // load constant value
    OpLoadLocal,  // load local variable
    OpLoadGlobal, // load global variable

    // accessor instructions
    OpGetf,
    OpSetf,
    OpAref,
    OpAset,
    OpAlength,

    // constructors
    OpCons,
    

    OpHalt,      // end of instruction sequence
  };

static inline Size opArgc( OpCode op )
{
  switch ( op )
    {
    case OpLoadConst: return 1;
    default:          return 0;
    }
}

#endif
