#include "util/hashing.h"

#include "object.h"
#include "type.h"
#include "value.h"

// external API
Value tag_float(Float x) {
  return double_to_word(x);
}

Value tag_arity(Arity x) {
  return (x & VAL_MASK) | ARITY_TAG;
}

Value tag_small(Small x) {
  return ((Value)x) | SMALL_TAG;
}

Value tag_bool(Boolean x) {
  return x ? TRUE : FALSE;
}

Value tag_glyph(Glyph x) {
  return ((Value)x) | GLYPH_TAG;
}

Value tag_ptr(Pointer x) {
  return ((Value)x) | PTR_TAG;
}

Value tag_fptr(FuncPtr x) {
  return ((Value)x) | FPTR_TAG;
}

Value tag_obj(void* x) {
  return ((Value)x) | OBJ_TAG;
}

extern struct Type ArityType, SmallType, UnitType, BooleanType, GlyphType,
  PointerType, FuncPtrType, FloatType;

Type* type_of_val(Value value) {
  Type* out;
  
  switch (value & TAG_MASK) {
    case ARITY_TAG: out = &ArityType;                  break;
    case SMALL_TAG: out = &SmallType;                  break;
    case NUL_TAG:   out = &UnitType;                   break;
    case BOOL_TAG:  out = &BooleanType;                break;
    case GLYPH_TAG: out = &GlyphType;                  break;
    case OBJ_TAG:   out = type_of_obj(as(Obj, value)); break;
    default:        out = &FloatType;                  break;
  }

  return out;
}

Type* type_of_obj(void* ptr) {
  assert(ptr != NULL);
  Obj*  obj = ptr;
  Type* out = obj->type;

  return out;
}

bool has_type_val(Value x, Type* type) {
  return is_instance(type_of(x), type);
}

bool has_type_obj(void* p, Type* type) {
  return is_instance(type_of(p), type);
}
