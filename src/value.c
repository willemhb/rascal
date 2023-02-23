#include "value.h"
#include "object.h"

#include "runtime.h"

#include "util/collections.h"

/* API */
Val tag_real(Real real) {
  return ((ValData)real).as_val;
}

Val tag_glyph(Glyph g) {
  return ((ValData)g).as_val | GLYPH_TAG;
}

Val tag_obj(void* obj) {
  return ((ValData)obj).as_val | OBJ_TAG;
}

Real as_real(Val val) {
  return ((ValData)val).as_real;
}

Glyph as_glyph(Val val) {
  return ((ValData)val).as_glyph;
}

void* as_obj(Val val) {
  return ((ValData)(val&VAL_MASK)).as_obj;
}

#define UNTAG(T, t)				\
  T* as_##t(Val val) {				\
    return (T*)as_obj(val);			\
  }

UNTAG(Sym, sym);
UNTAG(Func, func);
UNTAG(Bin, bin);
UNTAG(List, list);
UNTAG(Vec, vec);
UNTAG(Tuple, tuple);
UNTAG(Table, table);

#undef UNTAG

// type & value predicates ----------------------------------------------------
bool is_int(Val val) {
  if (!has_type(val, REAL))
    return false;

  Real real = as_real(val);

  return (real - (long)real) == 0;
}

bool is_byte(Val val) {
  if (!is_int(val))
    return false;

  long l = as_real(val);

  return l <= UINT8_MAX && l >= 0;
}

bool is_string(Val val) {
  return has_type(val, BIN) && has_flag(as_obj(val), ENCODED);
}

// generic untagging methods --------------------------------------------------
char* as_text(Val val) {
  if (has_type(val, SYM))
    return as_sym(val)->name;

  if (is_string(val))
    return as_bin(val)->array ? : "";

  return NULL;
}

// values array API -----------------------------------------------------------
void init_vals(Vals* vals) {
  vals->array = NULL;
  vals->count = 0;
  vals->cap   = pad_alist_size(0, 0);
}

void free_vals(Vals* vals) {
  deallocate(vals->array, vals->count, sizeof(Val));
  init_vals(vals);
}

void resize_vals(Vals* vals, uint n) {
  
  uint c = pad_alist_size(vals->count, n, vals->cap);

  if (c != vals->cap) {
    vals->array = reallocate(vals->array, vals->count, c, sizeof(Val), NOTUSED);
    vals->cap   = c;
  }
}

uint push_vals(Vals* vals, Val val) {
  resize_vals(vals, vals->count+1);

  vals->array[vals->count] = val;

  return vals->count++;
}

Val  pop_vals(Vals* vals) {
  assert(vals->count > 0);
  assert(vals->array != NULL);

  Val out = vals->array[--vals->count];

  resize_vals(vals, vals->count);

  return out;
}
