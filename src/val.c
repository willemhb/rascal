#include <stdarg.h>
#include <assert.h>

#include "val.h"
#include "obj.h"
#include "type.h"

/* C types */
#include "tpl/impl/alist.h"
ALIST(vals, val_t, val_t, pad_alist_size);

/* API */
val_type_t val_type(val_t x) {
  switch (tag_of(x)) {
  case SMALL:  return small_val;
  case BOOL:   return bool_val;
  case GLYPH:  return glyph_val;
  case NATIVE: return native_val;
  case OBJECT: return obj_val;
  default:     return real_val;
  }
}

type_t val_type_of(val_t val) {
  extern struct type_t RealType, SmallType, BoolType, GlyphType, NativeType;

  switch (val_type(val)) {
  case real_val:   return &RealType;
  case small_val:  return &SmallType;
  case bool_val:   return &BoolType;
  case glyph_val:  return &GlyphType;
  case native_val: return &NativeType;
  case obj_val:    return obj_type_of(as_obj(val));
  }

  rl_unreachable();
}

bool val_has_type(val_t val, type_t type) {
  if (type->isa)
    return type->isa(type, val);

  return val_type_of(val) == type;
}

bool is_val_type(val_t val, val_type_t valtype) {
  return val_type(val) == valtype;
}

/* convenience */
/* value tagging dispatch */
val_t value_to_value(val_t v) {
  /* no-op, allows already tagged values to be used. */
  return v;
}

val_t real_to_value(real_t r) {
  return as_val(r);
}

val_t small_to_value(small_t s) {
  return tag_val(s, SMALL);
}

val_t bool_to_value(bool_t b) {
  return tag_val(b, BOOL);
}

val_t glyph_to_value(glyph_t g) {
  return tag_val(g, GLYPH);
}

val_t native_to_value(native_t n) {
  return tag_val(n, NATIVE);
}

val_t object_to_value(void *o) {
  return tag_val((obj_t)o, OBJECT);
}

/* native functions */
#include "method.h"
#include "func.h"
#include "bool.h"

#include "tpl/impl/funcall.h"

/* validators */
eval_err_t guard_isap(int n, val_t *a) {
  (void)n;

  if (!is_type(a[1]))
    return arg_value_err;

  return no_err;
}

/* native handlers */
val_t native_isap(int n, val_t *a) {
  (void)n;

  val_t x = a[0];
  val_t t = a[1];

  type_t type = func_type(t);

  return has_type(x, type) ? TRUE : FALSE;
}

val_t native_idp(int n, val_t *a) {
  (void)n;

  if (a[0] == a[1])
    return TRUE;

  return FALSE;
}

/* initialization */
void val_init(void) {
  def_method("isa?", 2, false, guard_isap, native_isap);
  def_method("id?", 2, false, NULL, native_idp);
}
