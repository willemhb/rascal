#ifndef rascal_obj_object_h
#define rascal_obj_object_h

#include "obj/type.h"
#include "describe.h"

// base object methods & utilities go here
// memory api
object_t *new_obj( type_t *type, size_t n, void *data );
void      init_obj( object_t *self, type_t *type, size_t n, void *data );
void      obj_mark( object_t *obj );
void      obj_free( object_t *obj );

// convenience
#define rl_trace( x ) GENERIC_2( trace, x )

#endif
