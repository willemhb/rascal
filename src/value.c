#include "value.h"
#include "object.h"

#include "runtime.h"

#include "util/collections.h"

/* API */
#define TAG(type, Type, MASK, TAG)			\
  Val tag_##type(Type type) {				\
    return (((ValData)type).as_val & MASK) | TAG;	\
  }

TAG(real, Real,  WORD_MASK, REAL_TAG);
TAG(glyph, Glyph, VAL_MASK, GLYPH_TAG);
TAG(obj, Obj*, VAL_MASK, OBJ_TAG);
TAG(sym, Sym*, VAL_MASK, OBJ_TAG);
TAG(func, Func*, VAL_MASK, OBJ_TAG);
TAG(bin, Bin*, VAL_MASK, OBJ_TAG);
TAG(list, List*, VAL_MASK, OBJ_TAG);
TAG(table, Table*, VAL_MASK, OBJ_TAG);

#undef TAG

#define UNTAG(type, Type, MASK)			\
  Type as_##type(Val val) {			\
    return ((ValData)(val&MASK)).as_##type;	\
  }

UNTAG(real, Real,  WORD_MASK);
UNTAG(glyph, Glyph, VAL_MASK);
UNTAG(obj, Obj*, VAL_MASK);
UNTAG(sym, Sym*, VAL_MASK);
UNTAG(func, Func*, VAL_MASK);
UNTAG(bin, Bin*, VAL_MASK);
UNTAG(list, List*, VAL_MASK);
UNTAG(table, Table*, VAL_MASK);

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
  return has_type(val, BIN) && !!(as_bin(val)->flags & ENCODED);
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
  
  uint c = pad_alist_size(n, vals->cap);

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
