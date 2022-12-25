#include <string.h>

#include "arr.h"
#include "type.h"
#include "memory.h"

/* API */
/* internal */
size_t arr_size(obj_t self) {
  return base_size(self) + el_size(self) * arr_cap(self);
}

void init_arr(obj_t self, obj_type_t type, size_t n, void *ini) {
  arr_cap(self) = pad_method(type)(n, 0, 0);
  arr_len(self) = n;

  if (ini)
    memcpy(self, ini, n * el_size(type));
}

obj_t create_arr(obj_type_t type, size_t n, void *ini) {
  (void)ini;
 
  size_t p     = pad_method(type)(n, 0, 0);
  size_t base  = base_size(type);
  size_t total = base + p * el_size(type);
  return alloc(total) + base_offset(type);
}

obj_t resize_arr(obj_t self, size_t n) {
  size_t old_len = arr_len(self), old_cap = arr_cap(self);
  size_t p = pad_method(self)(old_len, n, old_cap);

  if (p != old_len)
    self = adjust_table(obj_start(self), base_size(self), old_len, p, el_size(self));

  return self;
}

/* convenience */
