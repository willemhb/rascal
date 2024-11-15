#include "val/sequence.h"

#include "vm/state.h"
#include "vm/heap.h"

/* External APIs */
Seq* mk_seq(void* x, bool m) {
  Seq* s = new_obj(&Vm, T_SEQ, m * MF_SEALED);

  init_seq(s, x);

  return s;
}

void init_seq(Seq* s, void* x) {
  // default values
  s->done = false;
  s->cseq = NULL;
  s->fst  = NOTHING;
  s->src  = x;
  s->off  = 0;

  SInitFn ini = sinitfn(x);

  assert(ini != NULL);      // Must explicitly set s->done
  preserve(&Vm, 1, tag(s)); // ini might create child sequences
  ini(s);
}

Val seq_first(Seq* s) {
  if ( s->fst == NOTHING ) {
    SFirstFn fst = sfirstfn(s->src); assert(fst != NULL);
    s->fst       = fst(s);
  }

  return s->fst;
}

Seq* seq_rest(Seq* s) {
  if ( s->done )
    return s = NULL;

  if ( s->sealed ) {
    s = unseal_obj(&Vm, s);
    s = seq_rest(s);
    s = seal_obj(&Vm, s);
  } else {
    s->fst      = NOTHING;
    SRestFn rst = srestfn(s->src); assert(rst != NULL);

    rst(s);
  }

  return s;
}
