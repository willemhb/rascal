#ifndef rl_obj_type_h
#define rl_obj_type_h

#include "def/vmtypes.h"

#include "vm/object.h"

/* commentary */

/* C types */
typedef struct vtable_t vtable_t;
typedef struct layout_t layout_t;


struct type_t
{
  object_t obj;
  string_t name;
  isa_fn_t isa;
  native_t constructor;
};

struct datatype_t
{
  type_t    type;

  /* basic layout information */
  vmtype_t vmtype;
  size_t   obsize;

  /* runtime methods */
  object_runtime_fn_t init;
  object_runtime_fn_t free;
  object_runtime_fn_t trace;
};

struct uniontype_t
{
  type_t       type;

  /* either a or b could be another union type */
  type_t      *left;
  type_t      *right;
};

/* globals */
extern datatype_t DataTypeType, UnionType;

extern uniontype_t TypeType, NoneType, AnyType;

/* API */
/* general accessors */
string_t get_type_name( type_t *type );
native_t get_type_constructor( type_t *type );
isa_fn_t get_type_isa( type_t *type ); 

/* union accessors */
type_t *get_union_left( uniontype_t *uniontype );
type_t *get_union_right( uniontype_t *uniontype );

/* datatype accessors */
/* runtime method accessors */
object_runtime_fn_t get_datatype_init( datatype_t *datatype );
object_runtime_fn_t get_datatype_trace( datatype_t *datatype );
object_runtime_fn_t get_datatype_free( datatype_t *datatype );

/* layout accessors */
size_t   get_datatype_obsize( datatype_t *datatype );
uint     get_datatype_flags( datatype_t *datatype );
vmtype_t get_datatype_vmtype( datatype_t *datatype );

/* isa? dispatch */
bool nonetype_isa( type_t *type, value_t x );
bool anytype_isa( type_t *type, value_t x );
bool datatype_isa( type_t *type, value_t x );
bool uniontype_isa( type_t *type, value_t x );

/* runtime */
void rl_obj_type_init( void );
void rl_obj_type_mark( void );
void rl_obj_type_cleanup( void );

/* convenience */
#define is_datatype( x )  (rl_typeof(x)==&TypeType)
#define as_datatype( x )  ((type_t*)((x)&PTRMASK))

#define gl_datatype_head  obj_init(&DataTypeType, sizeof(datatype_t), object_fl_static)
#define gl_uniontype_head obj_init(&UnionTypeType, sizeof(uniontype_t), object_fl_static)

#define gl_init_type( x ) init_object(&(x).type.obj)
#define gl_mark_type( x ) mark_object(&(x).type.obj)
#define gl_free_type( x ) free_object(&(x).type.obj)

#endif
