#include "labels.h"

#include "val/value.h"
#include "val/object.h"

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

Obj* val_as_obj(Val x) {
  assert(is_obj(x));

  return (Obj*)(x & DATA_BITS);
}

Obj* ptr_as_obj(void* p) {
  // assert(p != NULL);

  return p;
}

// type_of methods
Type type_of_val(Val x) {  
  switch (tag_bits(x)) {
    default:       return T_NUMBER;
    case CPOINTER: return T_CPOINTER;
    case OBJECT:   return as_obj(x)->tag;
    case LITTLE:   return x >> WTAG_SHIFT & WTYPE_MASK;
  }
}

Type type_of_obj(void* p) {
  assert(p != NULL);

  return ((Obj*)p)->tag;
}

// has_type methods
bool val_has_type(Val x, Type t) {
  return type_of(x) == t;
}

bool obj_has_type(void* x, Type t) {
  return type_of(x) == t;
}

// value predicates
bool is_nul(Val x) {
  return x == NUL;
}

bool is_bool(Val x) {
  return wtag_bits(x) == BOOL;
}

bool is_glyph(Val x) {
  return wtag_bits(x) == GLYPH;
}

bool is_num(Val x) {
  return is_big(x) || is_small(x);
}

bool is_big(Val x) {
  return (x & QNAN) != QNAN;
}

bool is_small(Val x) {
  return wtag_bits(x) == SMALL;
}

bool is_ptr(Val x) {
  return tag_bits(x) == CPOINTER;
}

bool is_obj(Val x) {
  return tag_bits(x) == OBJECT;
}

