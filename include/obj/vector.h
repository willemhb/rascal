#ifndef rl_obj_vector_h
#define rl_obj_vector_h

#include "obj/object.h"
#include "tpl/decl/array.h"

/* commentary */

/* C types */
struct vector_t { ARRAY(value_t); };

/* globals */
extern type_t VectorType;

/* API */
void resize_vector( vector_t *vector, size_t new_size );

/* runtime */
void rl_obj_vector_init( void );
void rl_obj_vector_mark( void );

/* convenience & utilities */
static inline bool      is_vector( value_t x ) { return rl_isa(x, &VectorType); }
static inline vector_t *as_vector( value_t x ) { return (vector_t*)as_object(x); }

#endif
