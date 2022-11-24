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
struct vector_t   { ARRAY(value_t); };
struct bytecode_t { ARRAY(ushort);  };
struct string_t   { ARRAY(char);    };

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

/* bytecode */
bytecode_t *make_bytecode( size_t n );
void        init_bytecode( bytecode_t *array, size_t n, ushort *data );
void        free_bytecode( bytecode_t *array );
size_t      resize_bytecode( bytecode_t *array, size_t new_count );

ushort      bytecode_ref( bytecode_t *array, long i );
ushort      bytecode_set( bytecode_t *array, long i, ushort x );
size_t      bytecode_add( bytecode_t *array, size_t n, ... );
ushort      bytecode_pop( bytecode_t *array, size_t n );
size_t      bytecode_cpy( bytecode_t *dst, ushort *src, size_t n );
size_t      bytecode_dup( ushort *dst, bytecode_t *src, size_t n );

/* bytecode */
string_t *make_string( size_t n );
void      init_string( string_t *array, size_t n, ushort *data );
void      free_string( string_t *array );
size_t    resize_string( string_t *array, size_t new_count );

ushort    string_ref( string_t *array, long i );
ushort    string_set( string_t *array, long i, ushort x );
size_t    string_add( string_t *array, size_t n, ... );
ushort    string_pop( string_t *array, size_t n );
size_t    string_cpy( string_t *dst, ushort *src, size_t n );
size_t    string_dup( ushort *dst, string_t *src, size_t n );

// convenience
#define resize_array( a, nc )			\
  _Generic((a),					\
	   vector_t*:resize_vector,		\
	   bytecode_t*:resize_bytecode	\
	   )((a), (nc))

#define array_ref( a, i )				\
  _Generic((a),						\
	   vector_t*:vector_ref,			\
	   bytecode_t*:bytecode_ref)((a), (i))

#define array_set( a, i, x )					\
  _Generic((a),							\
	   vector_t*:vector_ref,				\
	   bytecode_t*:bytecode_ref)((a), (i), (x))

#define array_add( a, n, ... )						\
  _Generic((a),								\
	   vector_t*:vector_add,					\
	   bytecode_t*:bytecode_add				\
	   )((a), (n) __VA_OPT__(,) __VA_ARGS__)

#define array_pop( a, n )						\
  _Generic((a),								\
	   vector_t*:vector_pop,					\
	   bytecode_t*:bytecode_pop)((a), (n))

#define array_cpy( a, b, n )					\
  _Generic((a),							\
	   vector_t*:vector_cpy,				\
	   bytecode_t*:bytecode_cpy)((a), (b), (n))

#define array_dup( d, s, n )					\
  _Generic((s),							\
	   vector_t*:vector_dup,				\
	   bytecode_t*:bytecode_dup)((d), (s), (n))

#endif
