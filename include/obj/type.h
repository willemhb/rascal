#ifndef rl_obj_type_h
#define rl_obj_type_h

#include "def/vmtypes.h"

#include "vm/object.h"

/* commentary */

/* C types */
typedef struct vtable_t vtable_t;
typedef struct layout_t layout_t;

struct vtable_t
{
  object_runtime_fn_t init;
  object_runtime_fn_t free;
  object_runtime_fn_t trace;
};

struct layout_t
{
  vmtype_t vmtype;
  uint     flags;
  size_t   obsize;
};

struct type_t
{
  object_t obj;
  string_t name;
};

struct datatype_t
{
  type_t    type;
  
  layout_t *layout;
  vtable_t *methods;
};

struct uniontype_t
{
  type_t       type;
  type_t      *member;
  uniontype_t *members;
};

/* globals */
extern datatype_t TypeType;

/* API */
/* accessors */
string_t get_type_name( type_t *type ); 

/* runtime method accessors */
object_runtime_fn_t get_datatype_init( datatype_t *datatype );
object_runtime_fn_t get_datatype_trace( datatype_t *datatype );
object_runtime_fn_t get_datatype_free( datatype_t *datatype );

/* layout accessors */
size_t   get_datatype_obsize( datatype_t *datatype );
uint     get_datatype_flags( datatype_t *datatype );
vmtype_t get_datatype_vmtype( datatype_t *datatype );

/* runtime */
void rl_obj_type_init( void );
void rl_obj_type_mark( void );
void rl_obj_type_cleanup( void );

/* convenience */
#define is_type( x )                 (rl_typeof(x)==&TypeType)
#define as_type( x )                 ((type_t*)((x)&PTRMASK))
#define get_type_objtype( x )        get_object_type((object_t*)(x))
#define get_type_objsize( x )        get_object_size((object_t*)(x))
#define get_type_objflags( x )       get_object_flags((object_t*)(x))
#define set_type_objflags( x, f )    set_object_flags((object_t*)(x), (f))
#define unset_type_objflags( x, f )  unset_object_flags((object_t*)(x), (f))
#define get_datatype_name( x )       get_type_name((type_t*)(x))
#define get_uniontype_name( x )      get_type_name((type_t*)(x))

#define gl_type_head      obj_init(&TypeType, sizeof(datatype_t), object_fl_static)
#define gl_init_type( x ) init_object(&(x).type.obj)
#define gl_mark_type( x ) mark_object(&(x).type.obj)
#define gl_free_type( x ) free_object(&(x).type.obj)

#endif
