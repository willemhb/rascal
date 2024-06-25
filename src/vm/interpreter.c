#include "vm/interpreter.h"

#include "val/function.h"

/* Globals */
IFrame IFrames[MAX_FRAMES];
Val    IVals[MAX_STACK];

IState Interpreter = {
  .fs = {
    .base=IFrames,
    .end =IFrames+MAX_FRAMES,
    .next=IFrames
  },

  .vs = {
    .base=IVals,
    .end =IVals+MAX_STACK,
    .next=IVals
  },

  .uv = NULL
};
/* Internal APIs */

/* External APIs */
void mark_iframe(IFrame* f) {
  mark(f->code);
  mark(f->hndl);
}

/* IState APIs */
void push_closure(IState* s, Closure* c, size_t n) {
  ifs_push(&s->fs, s->ex);

  s->ex.code = c;
  s->ex.hndl = NULL;
  s->ex.ip   = cls_ip(c);

  if ( s->ex.cp > -1 )
    s->ex.cp++;

  if ( s->ex.hp > -1 )
    s->ex.hp++;

  s->ex.bp = (s->vs.next-n) - s->vs.base;
}

void push_catch(IState* s, Closure* c, Closure* h) {
  
}
