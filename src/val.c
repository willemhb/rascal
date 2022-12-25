#include <stdarg.h>
#include <assert.h>

#include "val.h"
#include "obj.h"

/* C types */
#include "tpl/impl/alist.h"
ALIST(vals, val_t, val_t, pad_alist_size);

/* API */
val_type_t val_type(val_t x) {
  if ((x&QNAN) != QNAN)
    return real_val;

  return obj_val;
}

bool has_val_type(val_t x, val_type_t type) {
  return val_type(x) == type;
}

bool has_val_types(val_t x, size_t n, ...) {
  va_list va; va_start(va, n);
  bool out = false;

  for (size_t i=0; i<n; i++) {
    if ((out=has_val_type(x, va_arg(va, val_type_t))))
      break;
  }

  va_end(va);
  return out;
}

/* initialization */
void val_init( void ) {}
