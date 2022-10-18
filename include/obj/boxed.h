#ifndef rascal_boxed_h
#define rascal_boxed_h

#include "obj/object.h"
#include "describe.h"

// base type for all values with an extension type
struct boxed_t
{
  OBJ
  type_t *type;
};

// forward declarations
repr_t  box_repr( boxed_t* box );
type_t *box_type( boxed_t* box );
size_t  box_size( boxed_t* box );
size_t  box_unbox( boxed_t* box, size_t size, void *spc );
data_t  box_unwrap( boxed_t* box );
bool    val_is_box( value_t val );
bool    obj_is_box( object_t *obj );

#define is_box( x ) GENERIC_2( value_t, val_is_box, object_t*, obj_is_box, (x) )

// convenience
#define as_box( x ) ((boxed_t*)as_ptr(x))

#define BOX boxed_t box;

#endif
