#include "val.h"
#include "obj.h"


void mark_val(val_t val)
{
  if (is_obj(val))
    mark_obj(as_obj(val));
}

void mark_vals(val_t *vals, size_t n)
{
  for (size_t i=0; i<n; i++)
    mark_val(vals[i]);
}

void free_val(val_t val)
{
  if (is_obj(val))
    free_obj(as_obj(val));
}
