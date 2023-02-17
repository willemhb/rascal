#include "base/type.h"
#include "base/object.h"


/* API */
Type val_type_of(Val v) {
  switch (TAG_BITS(v)) {
    case INT_TAG:   return INT_TYPE;
    case UNIT_TAG:  return UNIT_TYPE;
    case BOOL_TAG:  return BOOL_TYPE;
    case GLYPH_TAG: return GLYPH_TYPE;
    case OBJ_TAG:   return obj_type_of(as_obj(v));
    default:        return REAL_TYPE;
  }
}


Type obj_type_of(Obj* o) {
  return o->type;
}

bool val_has_type(Val v, Type t) {
  return type_of(v) == t;
}

bool obj_has_type(Obj* o, Type t) {
  return o->type == t;
}
