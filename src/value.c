#include "base/value.h"
#include "base/object.h"

/* API */
#define TAG(type, Type, MASK, TAG)			\
  Val tag_##type(Type type) {				\
    return (((ValData)type).as_val & MASK) | TAG;	\
  }

TAG(cptr, void*, VAL_MASK, CPTR_TAG);
TAG(cstr, char*, VAL_MASK, CSTR_TAG);
TAG(real, Real,  WORD_MASK, REAL_TAG);
TAG(fixnum, FixNum, VAL_MASK, FIXNUM_TAG);
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

UNTAG(cptr, void*, VAL_MASK);
UNTAG(cstr, char*, VAL_MASK);
UNTAG(real, Real,  WORD_MASK);
UNTAG(fixnum, FixNum, VAL_MASK);
UNTAG(glyph, Glyph, VAL_MASK);
UNTAG(obj, Obj*, VAL_MASK);
UNTAG(sym, Sym*, VAL_MASK);
UNTAG(func, Func*, VAL_MASK);
UNTAG(bin, Bin*, VAL_MASK);
UNTAG(list, List*, VAL_MASK);
UNTAG(table, Table*, VAL_MASK);

#undef UNTAG

bool is_real(Val x) {
  return (x&QNAN) != QNAN;
}

bool is_unit(Val x) {
  return x == NUL;
}

#define ISA(type, getter, TAG)			\
  bool is_##type(Val val) {			\
    return getter(val) == TAG;			\
  }

ISA(cptr,   TAG_BITS, CPTR_TAG);
ISA(cstr,   TAG_BITS, CSTR_TAG);
ISA(fixnum, TAG_BITS, FIXNUM_TAG);
ISA(glyph,  TAG_BITS, GLYPH_TAG);
ISA(obj,    TAG_BITS, OBJ_TAG);

#undef ISA

#define ISA(t, T)				\
  bool val_is_##t(Val val) {			\
    return type_of(val) == T;			\
  }						\
  						\
  bool obj_is_##t(Obj* obj) {			\
    return obj->type == T;			\
  }

ISA(sym, SYM_TYPE);
ISA(stream, STREAM_TYPE);
ISA(func, FUNC_TYPE);
ISA(bin, BIN_TYPE);
ISA(list, LIST_TYPE);
ISA(table, TABLE_TYPE);
