#include <string.h>

#include "arr.h"
#include "type.h"
#include "memory.h"

/* globals */
bool isa_arr(type_t self, val_t val);

struct type_t ArrType = {
  .name="arr",
  .isa=isa_arr
};

/* API */
/* internal */
void init_arr(obj_t self, type_t type, size_t n, void *ini) {
  arr_head(self)->len = n;
  arr_head(self)->cap = type->pad(n, 0, 0);

  if (ini)
    memcpy(self, ini, n * type->el_size);
}

obj_t create_arr(type_t type, size_t n, void *ini) {
  (void)ini;
 
  size_t p     = type->pad(n, 0, 0);
  size_t base  = base_size(type);
  size_t total = base + p * type->el_size;
  return alloc(total) + type->head_size;
}

obj_t resize_arr(obj_t self, size_t n) {
  size_t old_len = arr_head(self)->len, old_cap = arr_head(self)->cap;
  size_t p = obj_type(self)->pad(old_len, n, old_cap);

  if (p != old_len) {
    type_t type = obj_type(self);
    size_t offs = type->head_size;
 
    self = adjust_table(self-offs, base_size(type), old_len, p, type->el_size) + offs;
    arr_head(self)->cap = p;
  }

  arr_head(self)->len = n;

  return self;
}

extern struct type_t VecType, CodeType;

bool isa_arr(type_t self, val_t val) {
  self = type_of(val);

  return self == &VecType || self == &CodeType;
}
