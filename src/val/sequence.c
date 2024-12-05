#include "val/sequence.h"

#include "vm/state.h"
#include "vm/heap.h"

/* globals */
void  trace_seq(State* vm, void* x);
void  seal_seq(State* vm, void* x, bool d);
bool  seq_empty(void* x);
Val   seq_first(void* x);
void* seq_rest(void* x);

VTable SeqVt = {
  .code    = T_SEQ,
  .name    = "seq",
  .obsize  = sizeof(Seq),
  .is_seq  = true,
  .tracefn = trace_seq,
  .sealfn  = seal_seq,
  .emptyfn = seq_empty,
  .firstfn = seq_first,
  .restfn  = seq_rest,
};

/* Runtime APIs && interfaces */
void trace_seq(State* vm, void* x) {
  Seq* s = x;

  mark(vm, s->cseq);
  mark(vm, s->src);
}

void seal_seq(State* vm, void* x, bool d) {
  Seq* s = x;

  if ( d && s->cseq )
    seal_seq(vm, s->cseq, d);
}

bool seq_empty(void* x) {
  Seq* s = x;

  return s->done;
}

Val seq_first(void* x) {
  Seq* s = x;

  if ( s->fst == NOTHING )
    s->f_fn(s);

  return s->fst;
}

void* seq_rest(void* x) {
  Seq* s = x;

  if ( !s->done ) {
    
  }

  return s;
}

/* Internal APIs */
#define assert_seq(vt) assert(vt->is_seq || vt->is_iseq)

void init_seq(Seq* s, void* x, VTable* vt) {
  s->done = false;
  s->cseq = NULL;
  s->fst  = NOTHING;
  s->src  = x;
  s->off  = 0;
  s->i_fn = vt->sinitfn;
  s->f_fn = vt->sfirstfn;
  s->r_fn = vt->srestfn;

  if ( s->i_fn ) {
    preserve(&Vm, 1, tag(s));
    s->i_fn(s);
  }
}

Seq* mk_seq(void* x, VTable* vt, bool s) {
  Seq* out = new_obj(&Vm, T_SEQ, s); init_seq(out, x, vt);

  return out;
}

/* External APIs */
// toplevel seq and iter APIs (iter is a mutable seq, used internally)
void* rl_seq(void* x) {
  void* r;

  if ( x == NULL )
    r = NULL;

  else {
    Obj* o     = x;
    VTable* vt = vtbl(o->tag); assert_seq(vt);

    if ( emptyfn(vt)(o) )
      r = NULL;

    else if ( vt->is_seq )
      r = o;

    else
      r = mk_seq(o, vt, true);
  }

  return r;
}

bool rl_empty(void* x) {
  bool r;

  if ( x == NULL )
    r = true;

  else {
    Obj* o = x;

    if ( o->tag == T_SEQ )
      r = ((Seq*)o)->done;

    else {
      VTable* vt = vtbl(o->tag); assert_seq(vt);

      r = emptyfn(vt)(o);
    }
  }

  return r;
}

Val rl_first(void* x) {
  Val r;

  if ( x == NULL )
    r = NUL;

  else {
    Obj* o = x;

    if ( o->tag == T_SEQ )
      r = seq_first(o);

    else {
      VTable* vt = vtbl(o->tag); assert_seq(vt);
      FirstFn ff = firstfn(vt);

      if ( ff )
        r = ff(o);

      else { // unlikely case of first being called on an ISeq object directly
        Seq* s = mk_seq(o, vt, true);
        r      = seq_first(s);
      }
    }
  }

  return r;
}

void* rl_rest(void* x) {
  void* r;

  if ( x == NULL )
    r = NULL;

  else {
    Obj* o = x;

    if ( o->tag == T_SEQ ) {
      if ( o->sealed ) {
        o = unseal_obj(&Vm, o);
        preserve(&Vm, 1, tag(o));
        r = rl_next(o);
        seal_obj(&Vm, o, false);
      } else {
        r = rl_next(o);
      }
    } else {
      VTable* vt = vtbl(o->tag); assert_seq(vt);
      RestFn rf  = restfn(vt);

      if ( rf )
        r = rf(o);

      else { // unlikely case of rest being called on an ISeq object directly
        Seq* s = mk_seq(o, vt, true);
        r      = seq_rest(s);
      }
    }
  }

  return r;
}

void* rl_iter(void* x) {
  void* r;

  if ( x == NULL )
    r = NULL;

  else {
    Obj* o     = x;
    VTable* vt = vtbl(o->tag); assert_seq(vt);

    if ( emptyfn(vt)(o) )
      r = NULL;

    else if ( vt->is_seq )
      r = o;

    else
      r = mk_seq(o, vt, false);
  }

  return r;
}

bool  rl_done(void* x) {
  // synonym for `empty?`
  return rl_empty(x);
}

void* rl_next(void* x) {
  void* r;

  if ( x == NULL )
    r = NULL;

  else {
    Obj* o = x;

    if ( o->tag == T_SEQ ) {
      assert(o->sealed == false);
      Seq* s = (Seq*)o;
      s->r_fn(s);

      if ( !s->done )
        r = s;
 
    } else {
      VTable* vt = vtbl(o->tag); assert_seq(vt);
      RestFn rf  = restfn(vt);

      if ( rf )
        r = rf(o);

      else { // unlikely case of rest being called on an ISeq object directly
        Seq* s = mk_seq(o, vt, false);
        s->r_fn(s);

        if ( !s->done )
          r = s;
      }
    }
  }

  return r;
}

#undef assert_seq
