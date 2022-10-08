#include "value.h"
#include "object.h"


void mark_value(value_t value)
{
  if (is_obj(value))
    mark_obj(as_obj(value));
}

void mark_values(value_t *values, size_t n)
{
  for (size_t i=0; i<n; i++)
    mark_value(values[i]);
}

void free_value(value_t value)
{
  if (is_obj(value))
    free_obj(as_obj(value));
}
