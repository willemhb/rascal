#include "data/value.h"
#include "data/object.h"

type_t valtype(value_t vx) {
  switch (vx & WTMASK) {
    case IMMTAG: return vx >> 32 & 0xff;
    case FIXTAG: return FIXNUM;
    case OBJTAG: return OBJECT;
    default:     return REAL;
  }
}

type_t val_typeof(value_t vx) {
  type_t vtype = valtype(vx);

  if (vtype == OBJECT)
    vtype = objtype(as_object(vx));

  return vtype;
}

usize val_sizeof(value_t vx) {
  type_t vtype = valtype(vx);
  usize out = 8;

  if (vtype == OBJECT)
    out = obj_sizeof(as_object(vx));

  return out;
}

bool val_isa(value_t vx, type_t tx) {
  if (tx == TOP)
    return true;

  if (tx == BOTTOM)
    return false;

  return val_typeof(vx) == tx;
}

void val_mark(value_t vx) {
  if (is_object(vx))
    obj_mark(as_object(vx));
}

void mark_values(usize n, value_t* vals) {
  for (usize i=0; i<n; i++)
    val_mark(vals[i]);
}
