#include <assert.h>

#include "value.h"
#include "object.h"

/* API */
flags32 val_flags(Val v) {
  assert(is_obj(v));

  return as_obj(v)->flags;
}
