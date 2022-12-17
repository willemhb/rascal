#ifndef rl_vm_object_h
#define rl_vm_object_h

#include "vm/value.h"

/* commentary

   common heap object type */

/* C types */
typedef enum object_type_t   object_type_t;
typedef enum object_layout_t object_layout_t;

enum object_layout_t
  {
    boxed_object_layout=1, /* has explicit type pointer */
    large_object_layout=2, /* includes size information */
    alloc_object_layout=4, /* is allocated (has invasive linked list of live objects) */
  };

enum object_type_t
  {
    invalid_object_type=-1,
    symbol_object_type=object_value_type+1,
    function_object_type,
    cons_object_type,
    string_object_type,
    vector_object_type,
    table_object_type,
    u16_array_object_type,
    record_object_type,

    control_object_type,
    closure_object_type,
    bytecode_object_type,
    namespace_object_type,
    variable_object_type,

    primitive_type_object_type,
    record_type_object_type,
    alias_type_object_type,
    union_type_object_type,
  };

struct rl_object_t
{
  object_type_t type;
  uchar black, gray, layout, flags, space[0];
};

/* globals */
extern rl_type_t *PrimitiveTypes[];

/* API */
/* runtime dispatchers */
rl_object_t *new_object( rl_type_t *type, size_t size );
rl_object_t *make_object( rl_type_t *type, size_t size );
void init_object( rl_object_t *self, rl_type_t *type, size_t size, bool is_alloc );
void trace_object( rl_object_t *self );
void free_object( rl_object_t *self );

/* predicates */
bool is_boxed( rl_object_t *self );
bool is_large( rl_object_t *self );
bool is_allocated( rl_object_t *self );

/* accessors */
rl_type_t *get_object_type( rl_object_t *self );
size_t get_object_size( rl_object_t *self );
rl_object_t *get_object_next( rl_object_t *self );

void set_object_type( rl_object_t *self, rl_type_t *type );
void set_object_size( rl_object_t *self, size_t size );
void set_object_next( rl_object_t *self, rl_object_t *next );

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
