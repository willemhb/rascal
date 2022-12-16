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

struct rl_object_t
{
  rl_object_t *next;
  ushort black;
  ushort gray;
  uint flags;
  size_t size;
  rl_datatype_t *type;
  uchar space[0];
};

/* globals */

/* API */
/* toplevel runtime methods */
rl_object_t *make_object( rl_datatype_t *datatype );
void init_object( rl_object_t *object );
void free_object( rl_object_t *object );
void mark_object( rl_object_t *object );

/* accessors */
rl_datatype_t *get_rl_object_type( rl_object_t *object );
size_t get_object_size( rl_object_t *object );
uint get_object_flags( rl_object_t *object );
uint set_object_flags( rl_object_t *object, uint fl );
uint unset_object_flags( rl_object_t *object, uint fl );

/* runtime */
void rl_vm_object_init( void );
void rl_vm_object_mark( void );
void rl_vm_object_cleanup( void );

/* convenience */
#define RL_OBJ_HEADER rl_object_t obj

#define is_object( x ) (tagof(x)==OBJECT)
#define as_object( x ) ((rl_object_t*)toptr(x))

#define tag_object( x ) tag((rl_object_t*)(x), OBJECT)

#define obj_init( _type, _size, _flags ) { .black=false, .gray=true, .size=_size, .type=_type, .flags=_flags }

#define object_init( x )  ((x)->type->init)
#define object_trace( x ) ((x)->type->trace)
#define object_free( x )  ((x)->type->free)

#endif
