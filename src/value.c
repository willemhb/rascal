#include <stdarg.h>
#include <assert.h>

#include "value.h"
#include "object.h"
#include "type.h"
#include "runtime.h"

/* C types */
#include "impl/alist.h"
ALIST(Values, Value, padAlistSize);

/* API */
RlType typeOf(Value x) {
  switch (tagOf(x)) {
  case SMALL:  return SmallType;
  case BOOL:   return BoolType;
  case UNIT:   return UnitType;
  case GLYPH:  return GlyphType;
  case OBJECT: return OBJ_TYPE(asObject(x));
  default:     return RealType;
  }
}

bool hasType(Value x, RlType type) {
  if (HasType[type])
    return HasType[type](x, type);

  return typeOf(x) == type;
}

bool hasTag(Value x, RlTag tag) {
  return tagOf(x) == tag;
}

void printValue(Value x) {
  RlType type = typeOf(x);

  if (Print[type])
    Print[type](x);

  printf("<%s>", TypeNames[type]);
}

Value realToValue(Real r) {
  return asValue(r);
}

Value smallToValue(Small s) {
  return tagValue(s, SMALL);
}

Value boolToValue(Bool b) {
  return b ? TRUE : FALSE;
}

Value glyphToValue(Glyph g) {
  return tagValue(g, GLYPH);
}

Value objectToValue(Object o) {
  assert(o);
  return tagValue(o, OBJECT);
}
