#include <assert.h>

#include "interp.h"
#include "object.h"

/* globals */
#define N_VALUES 8192
#define N_FRAMES 2048

Val   Values[N_VALUES];
Frame Frames[N_FRAMES];

struct Interp Interp = {
  .fp =Frames,
  .sp =Values
};

/* API */
Val* push(Val x) {
  assert(Interp.sp < Values+N_VALUES);
  Val* out = Interp.sp++;
  *out = x;
  return out;
}

Val* pushn(int n) {
  assert(Interp.sp+n < Values+N_VALUES);
  Val* out = Interp.sp;
  Interp.sp += n;
  return out;
}

Val* peep(int n) {
  if (n < 0)
    return Interp.sp+n;

  return Interp.fp->slots+n;
}

Val peek(int n) {
  return *peep(n);
}

Val pop(void) {
  assert(Interp.sp > Values);
  return *(--Interp.sp);
}

Val popn(int n) {
  assert(Interp.sp-n >= Values);
  Val out = peek(-1);
  Interp.sp -= n;
  return out;
}

void pushf(UserFn *closure, int n_args) {
  assert(Interp.fp < Frames+N_FRAMES);

  Frame* f = Interp.fp++;

  f->closure = closure;
  f->pc = (uint16*)closure->code->array;
  f->slots = Interp.sp - n_args;

  if (closure->vargs) {
    int n_vargs = n_args - closure->n_args;

    Val vargs = list(n_vargs, f->slots+closure->n_args);

    popn(n_vargs);
    push(vargs);
  }

  int to_push = num_locals(closure) - closure->n_args - closure->vargs;

  if (to_push)
    pushn(to_push);
}

void popf(void) {
  assert(Interp.fp > Frames);

  Val val = popn(num_locals((Interp.fp--)->closure));

  push(val);
}

bool is_captured(Frame *frame) {
  return frame->slots < Values || frame->slots >= Values+N_VALUES;
}
