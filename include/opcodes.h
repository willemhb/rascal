#ifndef rascal_opcode_h
#define rascal_opcode_h

#include "value.h"

typedef UShort OpCode;
typedef enum OpCodes OpCodes;

enum OpCodes
  {
    OpNothing,  // noop
    OpConstant, // load constant value
    OpHalt,     // end of instruction sequence
  };

static inline Size opArgc( OpCode op )
{
  switch ( op )
    {
    case OpConstant: return 1;
    default:         return 0;
    }
}

#endif
