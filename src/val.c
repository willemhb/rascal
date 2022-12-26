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
  if ((x&QNAN) != QNAN)
    return real_val;

  return obj_val;
}

type_t val_type_of(val_t val) {
  extern struct type_t RealType;

  switch (val_type(val)) {
  case real_val: return &RealType;
  case obj_val:  return obj_type_of(as_obj(val));
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

/* initialization */
void val_init(void) {}
