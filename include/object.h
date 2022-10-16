#ifndef rascal_object_h
#define rascal_object_h

#include "value.h"

// base object type
struct object_t
{
  union
  {
    value_t next;              // next allocated object
    
    struct                     // object tag fields
    {
      value_t repr   :  4;
      value_t        : 44;
      value_t gcbits :  2;
      value_t obtag  : 14;
    };
  };
};

// forward declarations
void      init_object( object_t *object, repr_t repr, bool isStatic );
void      mark_object( object_t *object );
void      unmark_object( object_t *object );
void      free_object( object_t *object );
size_t    obj_sizeof( object_t *object );

// convenience
#define as_object(val)    ((object_t*)as_ptr(val))

#define obj_next(val)     (as_object(val)->next)
#define obj_repr(val)     (as_object(val)->repr)
#define obj_gcbits(val)   (as_object(val)->gcbits)
#define obj_obtag(val)    (as_object(val)->obtag)

// GC flags
#define WHITE  0
#define GRAY   1
#define BLACK  2
#define STATIC 3

#endif
