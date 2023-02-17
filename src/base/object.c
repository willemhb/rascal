#include "base/object.h"


/* API */
bool is_obj(Val x) {
  return TAG_BITS(x) == OBJ_TAG;
}

Obj* as_obj(Val x) {
  return (Obj*)VAL_BITS(x);
}

Val obj_tag(Obj* o) {
  return (((ValData)o).as_val & VAL_MASK) | OBJ_TAG;
}

flags32 obj_flags(Obj* o) {
  return o->flags;
}
