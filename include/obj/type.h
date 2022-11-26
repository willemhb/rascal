#ifndef rl_obj_type_h
#define rl_obj_type_h

#include "vmtype.h"

#include "obj/object.h"

/* commentary */

/* C types */
struct type_data_t
{
  char    *name;

  size_t   obsize;
  size_t   elsize;
  vmtype_t vmtype;
  bool     stringp;
};

struct type_t
{
  struct object_t    obj;
  struct type_data_t data;
};

/* globals */
extern struct type_t TypeType;

/* API */

/* runtime */
void rl_obj_type_init( void );

/* convenience */
#define is_type( x ) (rl_typeof(x)==&TypeType.data)
#define as_type( x ) ((type_t)((x)&PTRMASK))

#endif
