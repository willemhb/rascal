#include "val/object.h"
#include "val/value.h"

// get the type of a value
extern Type SmallType, BooleanType, GlyphType, PointerType, FuncPtrType, RealType;

Type* type_of(Value x) {
  switch (x & TAG_MASK) {
    case SMALL_TAG: return &SmallType;
    case BOOL_TAG:  return &BooleanType;
    case GLYPH_TAG: return &GlyphType;
    case PTR_TAG:   return &PointerType;
    case FPTR_TAG:  return &FuncPtrType;
    case OBJ_TAG:   return as_obj(x)->type;
    default:        return &RealType;
  }
}

void mark_val(Value x) {
  if (is_obj(x))
    mark_obj(as_obj(x));
}

void trace_val(Value x) {
  if (is_obj(x))
    trace_obj(as_obj(x));
}
