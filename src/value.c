#include <stdarg.h>
#include "value.h"
#include "array.h"

data_type_t *typeof_value(value_t value)
{
  if ((value&QNAN) != QNAN)
    return ImmediateTypes[REAL];

  else if (!(value&SIGN))
    return ImmediateTypes[value>>40&255];

  else
    return typeof_object(as_object(value));
}

data_type_t *typeof_object(object_t *object)
{
  if (!object)
    return ImmediateTypes[NUL];

  return obj_type(object);
}

void mark_value(value_t value)
{
  if (is_object(value))
    mark_object(as_object(value));
}

void mark_values(value_t *values, size_t n)
{
  for (size_t i=0; i<n; i++)
    mark_value(values[i]);
}

void free_value(value_t value)
{
  if (is_object(value))
    free_object(as_object(value));
}

// utility array type implementations
OBJ_NEW(val_alist);

ARRAY_INIT(val_alist, value_t, ALIST_MINC);
ARRAY_RESIZE(val_alist, value_t, pad_alist_size, ALIST_MINC);
ARRAY_CLEAR(val_alist, value_t, ALIST_MINC);
ARRAY_WRITE(val_alist, value_t);
ARRAY_APPEND(val_alist, value_t);
ARRAY_POP(val_alist, value_t);
ARRAY_PUSH(val_alist, value_t);
ARRAY_MARK(val_alist, value_t);
ARRAY_FREE(val_alist, value_t);

OBJ_NEW(val_stack);

ARRAY_INIT(val_stack, value_t, STACK_MINC);
ARRAY_RESIZE(val_stack, value_t, pad_stack_size, STACK_MINC);
ARRAY_CLEAR(val_stack, value_t, STACK_MINC);
ARRAY_WRITE(val_stack, value_t);
ARRAY_APPEND(val_stack, value_t);
ARRAY_POP(val_stack, value_t);
ARRAY_PUSH(val_stack, value_t);
ARRAY_MARK(val_stack, value_t);
ARRAY_FREE(val_stack, value_t);
