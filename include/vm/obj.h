#ifndef rl_vm_obj_h
#define rl_vm_obj_h

#include "vm/val.h"

/* commentary

   common heap object type */

/* C types */
typedef enum obj_fl_t obj_fl_t;

enum obj_fl_t {
/* various internals are hidden before the object header. These flags indicate
   whether the given value is allocated and, therefore, whether it's been allocated. */

  allocated_object=1, /* includes invasive linked list of live objects. */
  aliased_object=2, /* user extension of builtin type (includes type pointer) */
};

struct obj_t
{
  obj_type_t object_type;

  uchar layout, flags, black, gray, space[0];
};

/* globals */
/* runtime dispatch tables */
extern void (*create_fn[])(type_t *type);
extern void (*init_fn[])(obj_t *self, type_t *type);
extern void (*trace_fn[])(obj_t *self);
extern void (*free_fn[])(obj_t *self);
extern size_t (*sizeof_fn[])(obj_t *self);

/* API */
obj_t *make_obj(obj_type_t type, int layout);

obj_type_t obj_type( obj_t *obj );
type_t *user_type( obj_t *obj );
bool has_obj_type( obj_t *obj, obj_type_t type );
void mark_obj( obj_t *obj );
size_t obj_size( obj_t *obj );

bool obj_is_alloc( obj_t *obj );
bool obj_is_user( obj_t *obj );

/* runtime */
/* toplevel initialization */
void rl_vm_object_init( void );
void rl_vm_object_mark( void );
void rl_vm_object_cleanup( void );

/* convenience */
#define OBJ obj_t obj

#endif
