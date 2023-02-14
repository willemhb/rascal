#ifndef interpreter_h
#define interpreter_h

#include "object.h"

/* C types */
typedef struct Frame Frame;

struct Interp {
  UpVal *open_upvals;
  Frame *ctl;
  Frame *fbase, *ftop, *fp;
  Val   *vbase, *vtop, *vp;
};

struct Frame {
  Frame   *caller;
  Closure *closure;
  uint16  *pc;
  Val     *base;
};

/* globals */
extern struct Interp Interp;

/* API */
void   interp_init(void);
void   init_interp(struct Interp *interp);
Val   *pushv(Val x);
Val    popv(void);
Frame *pushf(Closure *closure, int n_args);
Val    popf(void);

#endif
