#ifndef rascal_rlvm_h
#define rascal_rlvm_h

#include "object.h"

typedef struct Vm Vm;
typedef struct Buffer    Buffer;
typedef struct Control   Control;
typedef struct ByteCode  ByteCode;
typedef struct Vector    Vector;
typedef UShort           OpCode;

struct Control
{
  OBJECT;

  ByteCode *code;
  OpCode   *pc;
};

#define RASCAL_STACK_SIZE 2048

struct Vm
{
  OBJECT;

  // error handling state
  Bool     panic;
  Buffer  *error;

  // values stack
  Vector  *stack;

  // currently executing frame
  Control *control;

  Control  frames[RASCAL_STACK_SIZE];
};

// forward declarations
// control implementation
Control *makeControl( Void );
Int      freeControl( Control *control );
Void     initControl( Control *control, ByteCode *code );

#define isControl( x ) valueIsType(x, &ControlType)
#define asControl( x ) ((Control*)asObject(x))

// basic error handling
Bool recover( Void );
Void panic( const Char *fmt, ... );

// interpreter
Value rlExec( ByteCode *bytecode );

// initialization
Void rlVmInit( Void );

// globals
extern Vm TheVm;

extern Type VmType, ControlType;

#endif
