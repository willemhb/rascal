#include "object.h"
#include "value.h"

// array types
#include "describe.h"

ARRAY_TYPE(Values, Value);

Type valueType(Value value) {
  switch (value & TAG_MASK) {
    case NIL_TAG:  return UNIT;
    case BOOL_TAG: return BOOLEAN;
    case OBJ_TAG:  return OBJECT;
    default:       return NUMBER;
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
