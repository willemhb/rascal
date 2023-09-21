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

size_t sizeOfType(Type type) {
  size_t out;
  
  switch (type) {
    case NUMBER:  out = sizeof(Number);  break;
    case BOOLEAN: out = sizeof(Boolean); break;
    case UNIT:    out = sizeof(Value);   break;
    case ATOM:    out = sizeof(Atom);    break;
    case LIST:    out = sizeof(List);    break;
    case TUPLE:   out = sizeof(Tuple);   break;
    case MAP:     out = sizeof(Map);     break;
    case NODE:    out = sizeof(Node);    break;
    case LEAF:    out = sizeof(Leaf);    break;
    case CHUNK:   out = sizeof(Chunk);   break;
    case CLOSURE: out = sizeof(Closure); break;
    case UPVALUE: out = sizeof(UpValue); break;
    case NATIVE:  out = sizeof(Native);  break;
    case STREAM:  out = sizeof(Stream);  break;
    default:      out = 0;               break;
  }

  return out;
}
