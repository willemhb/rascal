#ifndef rl_obj_vector_h
#define rl_obj_vector_h

#include "obj/array.h"
#include "tpl/decl/array.h"

/* commentary */

/* C types */
struct vector_t
{
  ARRHEAD;
  value_t space[];
};

/* globals */
extern struct type_t VectorType;

/* API */
vector_t make_vector( size_t n, value_t *ini );
void     free_vector( vector_t vector );
size_t   resize_vector( vector_t vector, size_t new_count );

value_t  vector_ref( vector_t vector, long i );
value_t  vector_set( vector_t vector, long i, value_t x );
size_t   vector_add( vector_t vector, size_t n, ... );
value_t  vector_pop( vector_t vector, size_t n );

/* runtime */
void rl_obj_vector_init( void );

/* convenience */
#define is_vector( x ) (rl_typeof(x)==&VectorType.data)
#define as_vector( x ) ((vector_t)((x)&PMASK))

#define vector_header( x ) ((struct vector_t*)array_header((array_t)(x)))
#define vector_data( x )   ((vector_t)array_data((array_t)(x)))
#define vector_len( x )    (vector_header(x)->len)
#define vector_alloc( x )  (vector_header(x)->alloc)

#endif
