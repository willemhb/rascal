#ifndef rl_val_sequence_h
#define rl_val_sequence_h

#include "val/object.h"

/* Internal sequence interface object. */

/* C types */
struct Seq {
  HEADER;

  bool   done; // boolean flag indicating whether the sequence has any more values
  Seq*   cseq; // child sequence (for iterating through inductive objects)
  Val    fst;  // cached first element
  void*  src;  // source object
  size64 off;  // offset counter
};

/* External APIs */
#define is_seq(x) has_type(x, T_SEQ)
#define as_seq(x) ((Seq*)as_obj(x))

Seq*   mk_seq(void* x, bool m);
void   init_seq(Seq* s, void* x);
Val    seq_first(Seq* s);
Seq*   seq_rest(Seq* s);

#endif
