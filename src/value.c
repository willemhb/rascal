#include "object.h"
#include "value.h"

// array types
#include "describe.h"

ARRAY_TYPE(ValuesArray, Value);

Type valueType(Value value) {
  switch (value & TAGMASK) {
    case NILTAG:  return UNIT;
    case BOOLTAG: return BOOLEAN;
    case OBJTAG:  return OBJECT;
    default:      return NUMBER;
  }
}

Type rascalType(Value value) {
  Type type = valueType(value);

  if (type == OBJECT)
    type = objectType(AS_OBJ(value));

  return type;
}

Type objectType(Obj* object) {
  assert(object != NULL);

  return object->type;
}
