#ifndef rl_vm_interpreter_h
#define rl_vm_interpreter_h

#include "util/stack.h"

/* Runtime interpreter state types and APIs */
/* C types */
struct IFrame {
  // main 'registers'
  Closure*  code;
  Closure*  hndl;
  uint16_t* ip;
  size_t    bp;
  long      cp;
  long      hp;
};

struct IState {
  IfStack  fs;  // call stack
  ValStack vs;  // value stack
  IFrame   ex; // current execution state
  UpVal*   uv;  // open upvalues
};

/* External APIs */
/* IFrame APIs */
void mark_iframe(IFrame* f);

/* IState APIs */
void push_closure(IState* s, Closure* c, size_t n);

#endif
