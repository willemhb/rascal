#include "labels.h"
#include "runtime.h"

#include "val/value.h"
#include "val/object.h"
#include  "val/type.h"

#include "vm/state.h"

#include "util/hash.h"

/* Forward declarations */

/* APIs */
// tagging methods
Val tag_nul(Nul n) {
  (void)n;

  return NUL;
}

Val tag_bool(Bool b) {
  return b ? TRUE : FALSE;
}

Val tag_glyph(Glyph g) {
  return g | GLYPH;
}

Val tag_small(Small s) {
  return s | SMALL;
}

Val tag_num(Num n) {
  union {
    Num n;
    Val v;
  } u = { .n=n };

  return u.v;
}

Val tag_ptr(Ptr p) {
  return (Val)p | CPOINTER;
}

Val tag_obj(void* p) {
  return (Val)p | OBJECT;
}

// casting methods
Nul as_nul(Val x) {
  (void)x;

  return NULL;
}

Bool as_bool(Val x) {
  return x == TRUE;
}

Glyph as_glyph(Val x) {
  return x & WDATA_BITS;
}

Small as_small(Val x) {
  return x & WDATA_BITS;
}

Num as_num(Val x) {
  
  union {
    Num n;
    Val v;
  } u = { .v=x };

  return u.v;
}

Ptr as_ptr(Val x) {
  return (Ptr)(x & DATA_BITS);
}
