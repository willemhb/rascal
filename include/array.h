#ifndef rascal_array_h
#define rascal_array_h

#include "object.h"

// array types defined here, go in corresponding C file

// common array header
#define ARRAY(V)				\
  OBJHEAD;					\
  V      *data;					\
  size_t  capacity;				\
  size_t  count

// C types
struct vector_t       { ARRAY(value_t); };
struct instructions_t { ARRAY(ushort);  };
struct string_t       { ARRAY(char);    };

/* API */
/* vector */
vector_t *make_vector( size_t n );
void      init_vector( vector_t *array, size_t n, value_t *data );
size_t    resize_vector( vector_t *array, size_t new_count );

value_t   vector_ref( vector_t *array, long i );
value_t   vector_set( vector_t *array, long i, value_t x );
size_t    vector_add( vector_t *array, size_t n, ... );
value_t   vector_pop( vector_t *array, size_t n );
size_t    vector_cpy( vector_t *dst, value_t *src, size_t n );
size_t    vector_dup( value_t *dst, vector_t *src, size_t n );

/* instructions */
instructions_t *make_instructions( size_t n );
void            init_instructions( instructions_t *array, size_t n, ushort *data );
void            free_instructions( instructions_t *array );
size_t          resize_instructions( instructions_t *array, size_t new_count );

ushort          instructions_ref( instructions_t *array, long i );
ushort          instructions_set( instructions_t *array, long i, ushort x );
size_t          instructions_add( instructions_t *array, size_t n, ... );
ushort          instructions_pop( instructions_t *array, size_t n );
size_t          instructions_cpy( instructions_t *dst, ushort *src, size_t n );
size_t          instructions_dup( ushort *dst, instructions_t *src, size_t n );

// convenience
#define resize_array( a, nc )			\
  _Generic((a),					\
	   vector_t*:resize_vector,		\
	   instructions_t*:resize_instructions	\
	   )((a), (nc))

#define array_ref( a, i )				\
  _Generic((a),						\
	   vector_t*:vector_ref,			\
	   instructions_t*:instructions_ref)((a), (i))

#define array_set( a, i, x )					\
  _Generic((a),							\
	   vector_t*:vector_ref,				\
	   instructions_t*:instructions_ref)((a), (i), (x))

#define array_add( a, n, ... )						\
  _Generic((a),								\
	   vector_t*:vector_add,					\
	   instructions_t*:instructions_add				\
	   )((a), (n) __VA_OPT__(,) __VA_ARGS__)

#define array_pop( a, n )						\
  _Generic((a),								\
	   vector_t*:vector_pop,					\
	   instructions_t*:instructions_pop)((a), (n))

#define array_cpy( a, b, n )					\
  _Generic((a),							\
	   vector_t*:vector_cpy,				\
	   instructions_t*:instructions_cpy)((a), (b), (n))

#define array_dup( d, s, n )					\
  _Generic((s),							\
	   vector_t*:vector_dup,				\
	   instructions_t*:instructions_dup)((d), (s), (n))

#endif
