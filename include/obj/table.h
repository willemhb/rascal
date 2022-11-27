#ifndef rl_obj_table_h
#define rl_obj_table_h

#include "obj/object.h"

/* commentary */

/* C types */
struct table_t
{
  size_t  len;
  size_t  alloc;

  funcptr compare;

  struct object_t obj;

  object_t *data;
};

/* globals */

/* API */


#endif
