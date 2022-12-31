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
  case OBJECT: return obj_val;
  case BOOL:   return bool_val;
  default:     return real_val;
  }
}

type_t val_type_of(val_t val) {
  extern struct type_t RealType, SmallType, BoolType;

  switch (val_type(val)) {
  case real_val:  return &RealType;
  case small_val: return &SmallType;
  case bool_val:  return &BoolType;
  case obj_val:   return obj_type_of(as_obj(val));
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

/* native functions */
#include "func.h"
#include "bool.h"
#include "sym.h"
#include "native.h"

#include "tpl/impl/funcall.h"

/* validators */
func_err_t guard_isap(size_t nargs, val_t *args) {
  (void)nargs;

  if (!is_type(args[1]))
    return func_arg_value_err;

  return func_no_err;
}

/* native handlers */
val_t native_isap(size_t nargs, val_t *args) {
  (void)nargs;

  val_t x = args[0];
  val_t t = args[1];

  type_t type = func_head(t)->type;

  return has_type(x, type) ? TRUE : FALSE;
}

val_t native_idp(size_t nargs, val_t *args) {
  (void)nargs;

  if (args[0] == args[1])
    return TRUE;

  return FALSE;
}

/* initialization */
void val_init(void) {
  val_t isap = native("isa?", 2, false, guard_isap, NULL, native_isap);
  val_t idp  = native("id?", 2, false, NULL, NULL, native_idp);

  define("isa?", isap);
  define("id?", idp);
}
