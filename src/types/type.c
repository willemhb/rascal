#include "types/type.h"
#include "types/object.h"

/* API */
Type val_type_of(Val v) {
  switch (TAG_BITS(v)) {
    case SIGNAL_TAG:
    case CPTR_TAG:
    case CSTR_TAG:
    case FIXNUM_TAG: return FIXNUM_TYPE;
    case UNIT_TAG:   return UNIT_TYPE;
    case GLYPH_TAG:  return GLYPH_TYPE;
    case OBJ_TAG:    return obj_type_of(as_obj(v));
    default:         return REAL_TYPE;
  }
}

Type obj_type_of(Obj* o) {
  return o->type;
}

bool val_has_type(Val v, Type t) {
  switch (MetaTables[t].kind) {
    case BOTTOM: return false;
    case UNIT:   return v == NUL;
    case DATA:   return type_of(v) == t;
    case TOP:    return true;
  }

  unreachable();
}

bool obj_has_type(Obj* o, Type t) {
  switch(MetaTables[t].kind) {
    case BOTTOM: return false;
    case UNIT:   return false;
    case DATA:   return o->type == t;
    case TOP:    return true;
  }

  unreachable();
}

Mtable* val_mtable(Val x) {
  return MetaTables + type_of(x);
}

Mtable* obj_mtable(Obj* o) {
  return MetaTables + o->type;
}
