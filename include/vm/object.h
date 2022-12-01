#ifndef rl_vm_object_h
#define rl_vm_object_h

#include "vm/value.h"

/* commentary

   common heap object type */

/* C types */
typedef enum object_fl_t object_fl_t;

enum object_fl_t
  {
    object_fl_static=1,
  };

struct object_t
{
  object_t   *next;
  ushort      black;
  ushort      gray;
  uint        flags;
  size_t      size;
  datatype_t *type;
  uchar space[0];
};

/* globals */

/* API */
/* toplevel runtime methods */
object_t *make_object( datatype_t *datatype );
void      init_object( object_t *object );
void      free_object( object_t *object );
void      mark_object( object_t *object );

/* accessors */
datatype_t *get_object_type( object_t *object );
size_t      get_object_size( object_t *object );
uint        get_object_flags( object_t *object );
uint        set_object_flags( object_t *object, uint fl );
uint        unset_object_flags( object_t *object, uint fl );

/* runtime */
void rl_vm_object_init( void );
void rl_vm_object_mark( void );
void rl_vm_object_cleanup( void );

/* convenience */

#define is_object( x ) (tagof(x)==OBJECT)
#define as_object( x ) ((object_t*)toptr(x))

#define tag_object( x ) tag((object_t*)(x), OBJECT)

#define obj_init( _type, _size, _flags ) { .black=false, .gray=true, .size=_size, .type=_type, .flags=_flags }

#define object_init( x )  ((x)->type->methods->init)
#define object_trace( x ) ((x)->type->methods->trace)
#define object_free( x )  ((x)->type->methods->free)

#endif
