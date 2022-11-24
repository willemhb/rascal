#ifndef rl_obj_vector_h
#define rl_obj_vector_h

#include "obj/object.h"
#include "tpl/decl/array.h"

/* C types */
struct vector_t { ARRAY(value_t); };

/* globals */
extern type_t VectorType;

/* API */
void resize_vector( vector_t *vector, size_t new_size );

/* initialization */
void rl_obj_vector_init( void );

/* convenience & utilities */
#define as_vector( x ) ((vector_t*)as_obj(x))
#define is_vector( x ) value_is_type(x, &VectorType)

#endif
