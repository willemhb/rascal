#include "vm/obj/support/string.h"

#include "obj/type.h"

#include "util/string.h"

/* commentary */

/* C types */

/* globals */
void init_type( object_t *object );
void free_type( object_t *object );

bool datatype_isa( type_t *self, value_t x );
bool uniontype_isa( type_t *self, value_t x );
bool nonetype_isa( type_t *self, value_t x );
bool anytype_isa( type_t *self, value_t x );

datatype_t DataTypeType =
  {
    .type={ gl_datatype_head, "data-type", datatype_isa, NULL },

    .vmtype=vmtype_objptr,
    .obsize=sizeof(datatype_t),
    .init=init_type,
    .free=free_type,
    .trace=NULL
  };

datatype_t UnionTypeType =
  {
    .type={ gl_datatype_head, "union-type", datatype_isa, NULL },

    .vmtype=vmtype_objptr,
    .obsize=sizeof(uniontype_t),
    .init=init_type,
    .free=free_type,
    .trace=NULL
  };

uniontype_t TypeType =
  {
    .type={ gl_uniontype_head, "type", uniontype_isa, NULL },

    .left =&DataTypeType.type,
    .right=&UnionTypeType.type
  };

uniontype_t NoneType =
  {
    .type={ gl_uniontype_head, "none", nonetype_isa, NULL },

    .left=NULL,
    .right=NULL
  };

uniontype_t AnyType =
  {
    .type={ gl_uniontype_head, "any", anytype_isa, NULL },

    .left=NULL,
    .right=NULL
  };

/* API */
/* isa methods */
bool nonetype_isa( type_t *self, value_t x )
{
  (void)self;
  (void)x;
  return false;
}

bool anytype_isa( type_t *self, value_t x )
{
  (void)self;
  (void)x;
  return true;
}

bool datatype_isa( type_t *self, value_t x )
{
  return (type_t*)rl_typeof(x) == self;
}

bool uniontype_isa( type_t *self, value_t x )
{
  type_t *left = get_union_left((uniontype_t*)self),
    *right = get_union_right((uniontype_t*)self);

  return left->isa(left, x) || right->isa(right, x);
}

/* object runtime methods */
void init_type( object_t *object )
{
  type_t *type = (type_t*)object;

  if ( flagp(object->flags, object_fl_static) )
    type->name = make_string(strlen8(type->name), type->name);

  else
      type->name = make_string(strlen8("<type>"), "<type>");
}

void free_type( object_t *object )
{
  free_string(get_type_name((type_t*)object));
}

/* accessors */
#include "tpl/impl/record.h"
GET(type, name, string_t);
GET(type, isa, isa_fn_t);
GET(type, constructor, native_t);

GET(datatype, vmtype, vmtype_t);
GET(datatype, obsize, size_t);
GET(datatype, init, object_runtime_fn_t);
GET(datatype, free, object_runtime_fn_t);
GET(datatype, trace, object_runtime_fn_t);

GET(uniontype, left, type_t*);
GET(uniontype, right, type_t*);

/* runtime */
void rl_obj_type_init( void )
{
  gl_init_type(TypeType);
}

void rl_obj_type_mark( void )
{
  gl_mark_type(TypeType);
}

/* convenience */
