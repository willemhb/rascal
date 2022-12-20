#include <assert.h>

#include "val.h"
#include "obj.h"


/* API */
val_type_t val_type(val_t x) {
  if ((x&QNAN) != QNAN)
    return real_val;

  return obj_val;
}

rl_type_t rl_type(val_t x) {
  val_type_t vt = val_type(x);
  
  if (vt == obj_val)
    return (rl_type_t)obj_head(as_obj(x))->type;

  return (rl_type_t)vt;
}

bool has_val_type(val_t x, val_type_t type) {
  return val_type(x) == type;
}

bool has_rl_type(val_t x, rl_type_t type) {
  return rl_type(x) == type;
}

/* initialization */
void val_init( void ) {}
