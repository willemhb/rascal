#include "val.h"
#include "obj.h"


void mark_val(value_t value)
{
  if (is_obj(val))
    mark_obj(as_obj(val));
}

void mark_vals(value_t *vals, size_t n)
{
  for (size_t i=0; i<n; i++)
    mark_val(vals[i]);
}

void free_val(value_t value)
{
  if (is_obj(val))
    free_obj(as_obj(val));
}
