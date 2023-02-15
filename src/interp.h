#ifndef interpreter_h
#define interpreter_h

#include "object.h"

/* C types */
typedef struct Frame Frame;

struct Interp {
  Frame *fp;
  Val   *sp;
};

struct Frame {
  UserFn  *closure;
  uint16  *pc;
  Val     *slots;
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

void pushf(UserFn *closure, int n_args);
void popf(void);
bool is_captured(Frame *frame);

#endif
