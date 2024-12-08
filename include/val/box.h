#ifndef rl_val_box_h
#define rl_val_box_h

#include "val/object.h"

/* Object for representing boxed immediates. Actual VType and Type will correspond to boxed value. */
/* C types */
struct Box {
  HEADER;

  union {
    Nul   nul;
    Bool  boolean;
    Glyph glyph;
    Num   num;
    Ptr   ptr;
  };
};

/* Globals */

/* API */
Box*   rl_box(Val x);
size64 rl_unbox(Val x, void* spc, size64 n);

/* Initialization */

#endif
