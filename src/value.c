#include "util/hashing.h"

#include "object.h"
#include "value.h"

// array types
#include "tpl/describe.h"

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
    case BITS:    out = sizeof(Bits);    break;
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

char* nameOfType(Type type) {
  char* out;
  
  switch (type) {
    case NUMBER:  out = "Number";  break;
    case BOOLEAN: out = "Boolean"; break;
    case UNIT:    out = "Unit";    break;
    case ATOM:    out = "Atom";    break;
    case BITS:    out = "Bits";    break;
    case LIST:    out = "List";    break;
    case TUPLE:   out = "Tuple";   break;
    case MAP:     out = "Map";     break;
    case NODE:    out = "Node";    break;
    case LEAF:    out = "Leaf";    break;
    case CHUNK:   out = "Chunk";   break;
    case CLOSURE: out = "Closure"; break;
    case UPVALUE: out = "Upvalue"; break;
    case NATIVE:  out = "Native";  break;
    case STREAM:  out = "Stream";  break;
    default:      out = "Term";    break;
  }

  return out;
}

bool equalValues(Value x, Value y) {
  if (x == y)
    return true;

  Type xt = rascalType(x), yt = rascalType(y);

  if (xt != yt || xt < ATOM)
    return false;

  return equalObjects(AS_OBJ(x), AS_OBJ(y));
}

uint64_t hashValue(Value x) {
  Type xt = rascalType(x);

  if (xt < ATOM)
    return hashWord(x);

  return hashObject(AS_OBJ(x));
}
