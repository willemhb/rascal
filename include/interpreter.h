#ifndef rascal_interpreter_h
#define rascal_interpreter_h

#include "common.h"
#include "object.h"

// init parameters
#define N_STACK UINT16_COUNT
#define N_FRAME 8192

// C types
// activation record
typedef struct {
  Closure*  closure;
  uint16_t* ip;
  Value*    slots;
} CallFrame;

// execution state
typedef struct {
  Value*     sp;
  CallFrame* fp;
  UpValue*   openUpValues;
} Interpreter;

// globals
extern Value     theStack[N_STACK];
extern CallFrame theFrames[N_FRAME];

// external API
void   push(Value value);
Value  pop(void);
Value  peep(int n);
void   initInterpreter(Interpreter* interpreter, Value* stack, CallFrame* frames);

#endif
