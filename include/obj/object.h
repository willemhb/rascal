#ifndef rascal_object_h
#define rascal_object_h

#include "rl/value.h"

// function pointer types
typedef size_t (*obsize_t)( object_t *obj );
typedef void   (*obfree_t)( object_t *obj );
typedef void   (*obtrace_t)( object_t *obj );

// base object type
struct object_t
{
  union
  {
    value_t objnext;           // next allocated object
    struct                     // object tag fields
    {
      value_t objtag :  4;
      value_t        : 44;
      value_t gcbits :  2;
    };
  };
};

typedef struct ob_impl_t
{
  obsize_t  obsize;
  table_t  *slots;
} ob_impl_t;

// forward declarations
repr_t     obj_repr( object_t* obj );
type_t    *obj_type( object_t* obj );
size_t     obj_size( object_t* obj );
size_t     obj_unbox( object_t* obj, size_t size, void *spc );
rl_value_t obj_unwrap( object_t* obj );

// mark and free dispatch
void       obj_mark( object_t *obj );
void       obj_free( object_t *obj );

// convenience
#define OBJ object_t obj;

#define as_obj(x)   ((object_t*)as_ptr(x))
#define objnext(x)  (as_obj(x)->objnext)
#define objtag(x)   (as_obj(x)->objtag)
#define gcbits(x)   (as_obj(x)->gcbits)
#define nextobj(x)  ((object_t*)(objnext(x)&NXT_MASK))

// GC flags
#define WHITE   0
#define GRAY    1
#define BLACK   2
#define NOTRACE 3

#endif
