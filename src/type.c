#include "type.h"
#include "object.h"

type_t typeof_object(object_t *object)
{
  if (!object)
    return NUL;

  switch (object->next&TMASK)
    {
    case SYMBOL_TAG:   return SYMBOL;
    case VECTOR_TAG:   return VECTOR;
    case FUNCTION_TAG: return FUNCTION;
    default:           return ((type_t*)object)[2];
    }
}

type_t typeof_value(value_t value)
{
  switch ((value&TMASK))
    {
    case SMALL_TAG:    return value&7;
    case POINTER_TAG:  return POINTER;
    default:           return typeof_object(as_obj(value));
    }
}
