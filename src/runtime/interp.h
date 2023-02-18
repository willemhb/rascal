#ifndef runtime_interp_h
#define runtime_interp_h

#include "base/object.h"

/* C types */
typedef struct Frame Frame;

struct Interp {
  // call stack, values stack
  Frame* fp;
  Val*   sp;

  // global environment
  Table* toplevel;  // repl environment
  Table* module;    // currently executing module
  Table* modules;   // module cache
  Vec*   requiring; // save current form when a require form loads another module
};

struct Frame {
  Func*   closure;
  uint16* pc;
  Val*    slots;   // local arguments
};

/* globals */
extern struct Interp Interp;

/* API */
Val* push(Val x);
Val* pushn(int n);
Val* peep(int n);
Val peek(int n);
Val pop(void);
Val popn(int n);

#endif
