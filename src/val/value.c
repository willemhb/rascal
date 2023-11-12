#include "util/ieee754.h"

#include "val/map.h"
#include "val/table.h"
#include "val/type.h"
#include "val/object.h"
#include "val/value.h"

/* Globals */
MutDict ValueMeta = {
  .obj={
    .type =&MutDictType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|GRAY,
    .flags=FASTHASH,
  },
  .data=NULL,
  .cnt =0,
  .cap =0,
  .nts =0,
};

/* External APIs */
// get the type of a value
extern Type SmallType, BooleanType, GlyphType, PointerType, FuncPtrType, RealType;

Type* type_of_val(Value x) {
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

size_t size_of_val(Value val) {
  if (is_obj(val))
    return size_of_obj(as_obj(val));

  return type_of(val)->vtable->vsize;
}

bool has_type_val(Value val, Type* type) {
  return is_instance(type_of(val), type);
}

void mark_val(Value x) {
  if (is_obj(x))
    mark_obj(as_obj(x));
}

void trace_val(Value x) {
  if (is_obj(x))
    trace_obj(as_obj(x));
}

bool get_mfl_val(Value val, flags_t mfl) {
  assert(is_obj(val));

  return get_mfl_obj(as_obj(val), mfl);
}

bool set_mfl_val(Value val, flags_t mfl) {
  assert(is_obj(val));

  return set_mfl_obj(as_obj(val), mfl);
}

bool del_mfl_val(Value val, flags_t mfl) {
  assert(is_obj(val));

  return del_mfl_obj(as_obj(val), mfl);
}

bool get_fl_val(Value val, flags_t fl) {
  assert(is_obj(val));

  return get_fl_obj(as_obj(val), fl);
}

bool set_fl_val(Value val, flags_t fl) {
  assert(is_obj(val));

  return set_fl_obj(as_obj(val), fl);
}

bool del_fl_val(Value val, flags_t fl) {
  assert(is_obj(val));

  return set_fl_obj(as_obj(val), fl);
}

Dict* get_meta_dict_val(Value val) {
  if (is_obj(val))
    return get_meta_dict_obj(as_obj(val));

  Value d = mdict_get(&ValueMeta, val);

  if (is_obj(d))
    return as_dict(d);

  return &EmptyDict;
}

Dict* set_meta_dict_val(Value val, Dict* meta) {
  if (is_obj(val))
    return set_meta_dict_obj(as_obj(val), meta);

  mdict_set(&ValueMeta, val, tag(meta));

  return meta;
}

Value get_meta_val(Value val, Value key) {
  if (is_obj(val))
    return get_meta_obj(as_obj(val), key);

  Value d = mdict_get(&ValueMeta, val);

  if (d == NOTHING)
    return NOTHING;

  return dict_get(as_dict(d), key);
}

Value set_meta_val(Value val, Value key, Value kval) {
  if (is_obj(val))
    return set_meta_obj(as_obj(val), key, kval);

  Entry* e = mdict_intern(&ValueMeta, val);

  if (e->val == NOTHING)
    e->val = tag(&EmptyDict);

  Dict* d = as_dict(e->val);
  d = dict_set(d, key, kval);
  e->val = tag(d);

  return kval;
}

Dict* join_meta_val(Value val, Dict* meta) {
  if (is_obj(val))
    return join_meta_obj(as_obj(val), meta);

  Entry* e = mdict_intern(&ValueMeta, val);

  if (e->val == NOTHING)
    e->val = tag(&EmptyDict);

  Dict* d = as_dict(e->val);
  d = join_dicts(d, meta);
  e->val = tag(d);

  return d;
}


Value tag_float(Float f) {
  return double_to_word(f);
}

Value tag_small(Small s) {
  return (Value)s | SMALL_TAG;
}

Value tag_bool(Boolean b) {
  return b ? TRUE : FALSE;
}

Value tag_glyph(Glyph gl) {
  return (Value)gl | GLYPH_TAG;
}

Value tag_ptr(Pointer p) {
  return (Value)p | PTR_TAG;
}

Value tag_fptr(FuncPtr f) {
  return (Value)f | FPTR_TAG;
}

Value tag_obj(void* obj) {
  return (Value)obj | OBJ_TAG;
}
