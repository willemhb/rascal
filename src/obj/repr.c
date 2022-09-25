#include "obj/repr.h"

char *repr_name( repr_t *repr )
{
  return ((str_t*)repr->name)->vals;
}

char *type_name( type_t type )
{
  return repr_name(Reprs[type]);
}

