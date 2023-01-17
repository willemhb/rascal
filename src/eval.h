#ifndef rascal_eval_h
#define rascal_eval_h

#include "value.h"
#include "object.h"

/* C types */
typedef struct CallFrame CallFrame;

#define VALUES_STACK_SIZE 4096
#define FRAMES_STACK_SIZE 1024

struct CallFrame {
  CallFrame *caller;
  UserMethod callee;

  uint16    *ip;
  Value     *bp;
  Value     *sp;
};

struct Interpreter {
  bool    panicking;
  RlError error;

  CallFrame *currentFrame;

  Value      values[VALUES_STACK_SIZE];
  CallFrame  frames[FRAMES_STACK_SIZE];
};

/* API */
Value eval(Value x);
void  repl(void); 
Value exec(UserMethod callee);

#endif
