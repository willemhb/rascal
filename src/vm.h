#ifndef rascal_vm_h
#define rascal_vm_h

#include "chunk.h"

#define  STACK_MAX 256

struct VM
{
  Chunk    *chunk;
  uint16_t *ip;
  Value     stack[STACK_MAX];
  Value    *stackTop;
};

typedef enum
  {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
  } InterpretResult;

void            initVM( void );
void            freeVM( void );
InterpretResult interpret( const char *source );
void            push( Value value );
Value           pop( void );

#endif
