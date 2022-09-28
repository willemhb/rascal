#ifndef rascal_utils_table_h
#define rascal_utils_table_h

#include "core.h"

typedef struct ordmap_t
{
  union
  {
    int8_t  *data8;
    int16_t *data16;
    int32_t *data32;
  };

  arity_t    len;
  arity_t    cap;
} ordmap_t;

static inline getloc(ordmap_t *om, arity_t i)
{
  if (om->cap < INT8_MAX)
    return om->data8[i];

  if (om->cap < INT16_MAX)
    return om->data16[i];

  return om->data32[i];
}

#endif
