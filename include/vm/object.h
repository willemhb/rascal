#ifndef rl_vm_object_h
#define rl_vm_object_h

#include "vm/value.h"
#include "vm/type.h"

/* commentary

   common heap object type */

/* C types */
typedef enum layout_fl_t layout_fl_t;

enum layout_fl_t {
/* various internals are hidden before the object header. These flags indicate
   whether the given value is allocated and, therefore, whether it's been allocated. */

  allocated_object=1, /* includes invasive linked list of live objects. */
  dynamic_sized_object=2, /* object's size is not equal to the base size of its type */
  dynamic_arity_object=4, /* object includes non-standard arity information */
  annotated_object=8, /* object incudes additional metadata */
  typed_object=16, /* includes a non-standard type signature */
  borrowed_object=32, /* shares unboxed data with another object with the same root type (stores pointer to parent object). */
  aliased_object=64, /* user extension of builtin type (includes constructor pointer) */

  /* miscellaneous other flags (not stored before the header) */
  forwarded_object_layout=128, /* object had to be moved for some reason - pointer to new location stored at *(void**)o->space */
};

struct rl_object_t
{
  object_type_t object_type;

  uchar layout, flags, black, gray, space[0];
};

/* globals */
/* runtime dispatch tables */
extern void (*create_fn[])(type_t *type, int flags, int layout, ...);
extern void (*init_fn[])(rl_object_t *self, type_t *type, int flags, int layout, ...);
extern void (*trace_fn[])(rl_object_t *self);
extern void (*free_fn[])(rl_object_t *self);

/* API */
rl_object_t *make_object(object_type_t type, int layout);

object_type_t object_type( rl_object_t *object );
type_t *user_type( rl_object_t *object );
bool has_object_type( rl_object_t *object, object_type_t type );
void mark_object( rl_object_t *object );
size_t object_size( rl_object_t *object );

rl_object_t **object_next_object( rl_object_t *object );
size_t *object_dynamic_size( rl_object_t *object );
size_t *object_dynamic_arity( rl_object_t *object );
rl_object_t **object_metadata( rl_object_t *object );
rl_object_t **object_lender( rl_object_t *object );
type_t **object_alias_type( rl_object_t *object );

bool object_is_allocated( rl_object_t *object );
bool object_has_dynamic_size( rl_object_t *object );
bool object_has_dynamic_arity( rl_object_t *object );
bool object_has_metadata( rl_object_t *object );
bool object_is_borrowed( rl_object_t *object );
bool object_is_aliased( rl_object_t *object );

/* runtime */
/* toplevel initialization */
void rl_vm_object_init( void );
void rl_vm_object_mark( void );
void rl_vm_object_cleanup( void );

/* convenience */
#define RL_OBJ_HEADER rl_object_t obj

#define is_object( x ) has_value_type(x, object_value)

#endif
