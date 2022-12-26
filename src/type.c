#include <stdarg.h>
#include <assert.h>

#include "type.h"

#include "obj.h"
#include "val.h"

/* globals */
bool any_isa(type_t self, val_t val);

struct type_t AnyType = {
  .name="any",
  .isa=any_isa
};

/* API */
/* external */
size_t base_size(type_t type) {
  return type->head_size + type->body_size;
}

size_t alloc_size(type_t type, size_t n) {
  size_t out = base_size(type);

  if (type->el_size==0)
    return out;

  out += (type->pad ? type->pad(n, 0, 0) : n) * type->el_size;

  return out;
}

/* internal */
bool any_isa(type_t self, val_t val) {
  (void)self;
  (void)val;

  return true;
}

/* initialization */
void type_init(void) {}
