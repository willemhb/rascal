#ifndef rascal_object_h
#define rascal_object_h

#include <stdio.h>

#include "rascal.h"

// core object types defined in here
#define OBJHEAD object_t obj

#define OBJINIT(type, size, allocated)				\
  { NULL, NUL, (type), (size), true, false, (allocated), 0 }

struct object_t
{
  object_t *next;           // live object list
  value_t   _meta;          // association list of object metadata
  type_t   *type;           // this object's type
  uint      size;           // object's total size
  uchar     gray;
  uchar     black;
  uchar     allocated;
  uchar     flags;
  uchar     space[0];     // beginning of object's own data
};

/* globals */
// external
object_t *make_obj( type_t *type );
void init_obj( object_t *obj, size_t n, void *ini );
void free_obj( object_t *obj );

/* convenience and utilities */
#define tag_obj( x ) tag_ptr( x, OBJECT )

#define as_type( x ) ((type_t*)as_object(x))
#define is_type( x ) value_is_type(x, &TypeType)

#endif
