#include "value.h"

ValType val_type(Val x) {
  switch (TAG_BITS(x)) {
  case SMALL_TAG: return SMALL_VAL;
  case UNIT_TAG:  return UNIT_VAL;
  case BOOL_TAG:  return BOOL_VAL;
  case GLYPH_TAG: return GLYPH_VAL;
  case OBJ_TAG:   return OBJ_VAL;
  default:        return REAL_VAL;
  }
}

Val mk_glyph(Glyph g) {
  return ((ValData)g).as_val | GLYPH_TAG;
}

Val mk_bool(Bool b) {
  return b ? TRUE_VAL : FALSE_VAL;
}

Val mk_small(Small s) {
  return ((ValData)s).as_val | SMALL_TAG;
}

Val mk_real(Real r) {
  return ((ValData)r).as_val;
}
