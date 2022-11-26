#ifndef rl_obj_array_h
#define rl_obj_array_h

#include "obj/object.h"

/* commentary */

/* C types */
struct array_t
{
  size_t len;
  size_t alloc;
  bool   inlinep;
  bool   dynamicp;
  struct object_t obj;
};

/* globals */

/* API */
array_t make_array( type_t type, size_t n, void *ini );
size_t  resize_array( array_t array, size_t new_count );
void    free_array( array_t array );

/* convenience */
size_t pad_array_size( size_t new_count, size_t old_cap );

static inline struct array_t *array_header( array_t array )
{
  return (struct array_t*)(array - sizeof(struct array_t));
}

static inline array_t array_data( array_t array )
{
  if ( array_header(array)->inlinep )
    return array;

  return *(void**)array;
}

#define array_len( x )      (array_header(x)->len)
#define array_alloc( x )    (array_header(x)->alloc)
#define array_inlinep( x )  (array_header(x)->inlinep)
#define array_dynamicp( x ) (array_header(x)->dynamicp)

#endif

