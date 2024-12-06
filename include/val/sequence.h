#ifndef rl_val_sequence_h
#define rl_val_sequence_h

#include "val/object.h"

/* Internal sequence interface object. */

/* C types */
struct Seq {
  HEADER;

  bool     done;  // boolean flag indicating whether the sequence has any more values
  Seq*     cseq;  // child sequence (for iterating through inductive objects)
  Val      fst;   // cached first element
  void*    src;   // source object
  size64   off;   // offset counter

  // interface functions cached from VTable
  SInitFn  i_fn;
  SFirstFn f_fn;
  SRestFn  r_fn;
};

/* External APIs */
#define is_seq(x) has_type(x, T_SEQ)
#define as_seq(x) ((Seq*)as_obj(x))

// toplevel seq and iter APIs (iter is a mutable seq, used internally)
void*  rl_seq(void* x);
bool   rl_empty(void* x);
Val    rl_first(void* x);
void*  rl_rest(void* x);

void*  rl_iter(void* x);
bool   rl_done(void* x);    // synonym for rl_empty
void*  rl_next(void* s);

Seq*   mk_seq(void* x, bool s);

#endif
