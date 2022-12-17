#ifndef rl_vm_object_h
#define rl_vm_object_h

#include "vm/value.h"

/* commentary

   common heap object type */

/* C types */
typedef enum object_t object_t;

enum object_t {
  no_object=-1,
  nul_object=nul_value,
  symbol_object=real_value+1, function_object, cons_object,
  string_object, vector_object, dict_object, set_object,
  u16_array_object, record_object, control_object, closure_object,
  bytecode_object, namespace_object, variable_object
};

struct rl_object_t
{
  rl_type_t *constructor;
  object_t object_type;
  ushort flags;
  uchar blac, gray, space[0];
};

/* globals */

/* API */
object_t object_type( rl_object_t *object );
void mark_object( rl_object_t *object );

/* runtime */
/* toplevel initialization */
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
