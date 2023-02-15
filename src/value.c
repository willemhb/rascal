#include "value.h"
#include "object.h"

ValType val_type(Val x) {
  switch (TAG_BITS(x)) {
  case SMALL_TAG: return SMALL_VAL;
  case FIX_TAG:   return FIX_VAL;
  case UNIT_TAG:  return UNIT_VAL;
  case BOOL_TAG:  return BOOL_VAL;
  case GLYPH_TAG: return GLYPH_VAL;
  case OBJ_TAG:   return OBJ_VAL;
  default:        return REAL_VAL;
  }
}

char *type_name(Val x) {
  static char *ValTypeNames[] = {
    [REAL_VAL] = "real",  [FIX_VAL] = "fix",
    [SMALL_VAL] = "small", [UNIT_VAL]  = "unit",
    [BOOL_VAL] = "bool", [GLYPH_VAL] = "glyph",
    [OBJ_VAL] = "obj"
  };

  ValType type = val_type(x);

  if (type == OBJ_VAL)
    return obj_type_name(as_obj(x));

  return ValTypeNames[type];
}

bool is_val_type(Val x, ValType type) {
  return val_type(x) == type;
}

#define VAL_TYPE(_type, _TYPE)			\
  bool is_##_type(Val x) {			\
    return is_val_type(x, _TYPE##_VAL);		\
  }

VAL_TYPE(real, REAL);
VAL_TYPE(fix, FIX);
VAL_TYPE(small, SMALL);
VAL_TYPE(unit, UNIT);
VAL_TYPE(bool, BOOL);
VAL_TYPE(glyph, GLYPH);
VAL_TYPE(obj, OBJ);

#undef VAL_TYPE

bool is_num(Val x) {
  ValType type = val_type(x);

  return type >= REAL_VAL && type <= SMALL_VAL;
}

#define TAG_VAL(_type, _Type, _MASK, _TAG)		\
  Val mk_##_type(_Type x) {				\
    return (((ValData)x).as_val & _MASK) | _TAG;	\
  }

TAG_VAL(glyph, Glyph, VAL_MASK, GLYPH_TAG);
TAG_VAL(fix, Fix, VAL_MASK, FIX_TAG);
TAG_VAL(small, Small, VAL_MASK, SMALL_TAG);
TAG_VAL(bool, Bool, VAL_MASK, BOOL_TAG);
TAG_VAL(real, Real, WORD_MASK, REAL_TAG);
TAG_VAL(obj, Obj*, VAL_MASK, OBJ_TAG);

#undef TAG_VAL

#define UNTAG_VAL(_type, _Type, _MASK)		\
  _Type as_##_type(Val x) {			\
    return ((ValData)(x&_MASK)).as_##_type;	\
  }

UNTAG_VAL(real, Real, WORD_MASK);
UNTAG_VAL(fix, Fix, VAL_MASK);
UNTAG_VAL(small, Small, VAL_MASK);
UNTAG_VAL(bool, Bool, VAL_MASK);
UNTAG_VAL(glyph, Glyph, VAL_MASK);
UNTAG_VAL(obj, Obj*, VAL_MASK);

#undef UNTAG_VAL
