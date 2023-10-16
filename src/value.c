#include "util/hashing.h"

#include "object.h"
#include "type.h"
#include "value.h"

// external API
Value tagFloat(Float x) {
  return doubleToWord(x);
}

Value tagArity(Arity x) {
  return (x & VAL_MASK) | ARITY_TAG;
}

Value tagSmall(Small x) {
  return ((Value)x) | SMALL_TAG;
}

Value tagBoolean(Boolean x) {
  return x ? TRUE : FALSE;
}

Value tagGlyph(Glyph x) {
  return ((Value)x) | GLYPH_TAG;
}

Value tagObj(void* x) {
  return ((Value)x) | OBJ_TAG;
}

Type* typeOfVal(Value value) {
  Type* out;
  
  switch (value & TAG_MASK) {
    case ARITY_TAG: out = &ArityType;               break;
    case SMALL_TAG: out = &SmallType;               break;
    case NUL_TAG:   out = &UnitType;                break;
    case BOOL_TAG:  out = &BooleanType;             break;
    case GLYPH_TAG: out = &GlyphType;               break;
    case OBJ_TAG:   out = typeOfObj(AS_PTR(value)); break;
    default:        out = &FloatType;               break;
  }

  return out;
}

Type* typeOfObj(void* ptr) {
  assert(ptr != NULL);
  Obj*  obj = ptr;
  Type* out = obj->type;

  return out;
}

static bool isInstance(Type* vt, Type* type) {
  bool out;
  
  switch (type->kind) {
    case BOTTOM_KIND:     out = false;      break;
    case TOP_KIND:        out = true;       break;
    case DATA_TYPE_KIND:  out = vt == type; break;
    case UNION_TYPE_KIND:
      if (type->left)
        out = isInstance(vt, type->left);

      if (!out && type->right)
        out = isInstance(vt, type->right);

      break;
    case ABSTRACT_TYPE_KIND:
      vt = vt->parent;

      for (;vt != &AnyType && !out; vt=vt->parent)
        out = vt == type;
      break;
  }

  return out;
}

bool hasTypeVal(Value x, Type* type) {
  return isInstance(typeOf(x), type);
}

bool hasTypeObj(void* p, Type* type) {
  return isInstance(typeOf(p), type);
}
