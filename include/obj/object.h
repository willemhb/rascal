#ifndef rl_obj_object_h
#define rl_obj_object_h

#include "vm/value.h"

/* commentary

   common heap object type */

/* C types */
struct object_t
{
  size_t size;
  type_t type;
  uchar space[0];
};

/* globals */

/* API */
object_t make_object( type_t type );
void     free_object( object_t object );

/* runtime */
void rl_obj_object_init( void );

/* convenience */
#define is_object( x ) (((x)&TAGMASK) == OBJECT)
#define as_object( x ) ((object_t)((x)&PTRMASK))
#define obj_head( x )  ((struct object_t*)((x)-sizeof(struct object_t)))

#endif
