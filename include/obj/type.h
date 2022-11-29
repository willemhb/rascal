#ifndef rl_obj_type_h
#define rl_obj_type_h

#include "def/datatypes.h"

#include "vm/object.h"

/* commentary */

/* C types */
typedef struct vtable_t vtable_t;
typedef struct layout_t layout_t;
typedef enum type_fl_t  type_fl_t;

struct vtable_t
{
  void (*init)(object_t *object);
  void (*free)(object_t *object);
  void (*trace)(object_t *object);
};

struct layout_t
{
  vmtype_t vmtype;
  uint     flags;
  size_t   obsize;
};

enum type_fl_t
  {
    type_fl_static=1,
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

/* runtime */
void rl_obj_type_init( void );
void rl_obj_type_mark( void );

/* convenience */
#define is_type( x )     (rl_typeof(x)==&TypeType)
#define as_type( x )     ((type_t*)((x)&PTRMASK))
#define type_name( x )   (((type_t*)(x))->name)
#define type_init( x )   (((datatype_t*)(x))->methods->init)
#define type_trace( x )  (((datatype_t*)(x))->methods->trace)
#define type_free( x )   (((datatype_t*)(x))->methods->free)

#endif
