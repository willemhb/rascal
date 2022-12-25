#include <stdarg.h>
#include <assert.h>

#include "type.h"

#include "obj.h"
#include "val.h"

/* globals */
struct dtype_t Type[num_types];

/* API */
/* generics */
#include "tpl/impl/generic.h"
rl_type_t METHOD(val, rl_type)(val_t val) {
  if (is_obj(val))
    return rl_type(as_obj(val));

  return (rl_type_t)val_type(val);
}

rl_type_t METHOD(obj, rl_type)(obj_t obj) {
  if (obj)
    return (rl_type_t)obj_type(obj);

  return nul_type;
}

bool METHOD(val, has_rl_type)(val_t val, rl_type_t type) {
  return rl_type(val) == type;
}

bool METHOD(obj, has_rl_type)(obj_t obj, rl_type_t type) {
  return rl_type(obj) == type;
}

bool METHOD(val, has_rl_types)(val_t val, size_t n, ...) {
  va_list va; va_start(va, n);
  bool out = false;

  for (size_t i=0; i<n; i++) {
    if ((out=has_rl_type(val, va_arg(va, rl_type_t))))
      break;
  }

  va_end(va);
  return out;
}

bool METHOD(obj, has_rl_types)(obj_t obj, size_t n, ...) {
  va_list va; va_start(va, n);
  bool out = false;

  for (size_t i=0; i<n; i++) {
    if ((out=has_rl_type(obj, va_arg(va, rl_type_t))))
      break;
  }

  va_end(va);
  return out;
}

dtype_t METHOD(dtype, dtype_for)(dtype_t dtype) {
  assert(dtype);
  return dtype;
}

dtype_t METHOD(obj_type, dtype_for)(obj_type_t obj_type) {
  return &Type[obj_type];
}

dtype_t METHOD(rl_type, dtype_for)(rl_type_t rl_type) {
  return &Type[rl_type];
}

dtype_t METHOD(obj, dtype_for)(obj_t obj) {
  return dtype_for(obj_type(obj));
}

dtype_t METHOD(val, dtype_for)(val_t val) {
  return dtype_for(rl_type(val));
}

dtype_t METHOD(obj, dtype_of)(obj_t obj) {
  return dtype_for(obj);
}

dtype_t METHOD(val, dtype_of)(val_t val) {
  return dtype_for(val);
}

size_t METHOD(dtype, base_size_for)(dtype_t dtype) {
  return head_size_for(dtype) + body_size_for(dtype);
}

size_t METHOD(obj_type, base_size_for)(obj_type_t obj_type) {
  return base_size_for(dtype_for(obj_type));
}

size_t METHOD(rl_type, base_size_for)(rl_type_t rl_type) {
  return base_size_for(dtype_for(rl_type));
}

size_t METHOD(obj, base_size_for)(obj_t obj) {
  return base_size_for(dtype_for(obj));
}

size_t METHOD(val, base_size_for)(val_t val) {
  return base_size_for(dtype_for(val));
}
