#ifndef rascal_invoke_h
#define rascal_invoke_h

#include <stdarg.h>

#include "rl/value.h"

// signatures for builtin function types
// those that can be overridden must only take and return rl_data_t

// internal functions can't be overridden
typedef bool      (*isa_t)( type_t *self, value_t value );
typedef bool      (*has_t)( type_t *self, type_t *other );
typedef void      (*free_t)( object_t *object );
typedef void      (*trace_t)( object_t *object );
typedef int       (*cvinit_t)( type_t *type, value_t val, void *spc );
typedef int       (*obinit_t)( type_t *type, object_t *obj, size_t n, value_t *args );
typedef object_t  (*create_t)( size_t n );
typedef array_t  *(*resize_t)( array_t *self, size_t newl );
typedef size_t    (*sizeof_t)( object_t *self );

// builtin functions
typedef value_t (*thunk_t)  ( void );
typedef value_t (*unary_t)  ( value_t x );
typedef value_t (*binary_t) ( value_t x, value_t y );
typedef value_t (*ternary_t)( value_t x, value_t y, value_t z );
typedef value_t (*nary_t)   ( value_t *args, size_t n );

// value api methods
typedef ord_t    (*compare_t)( rl_data_t x, rl_data_t y );
typedef hash_t   (*hashof_t)(  rl_data_t x );
typedef value_t  (*read_t)(    rl_data_t stream, rl_data_t dispatch );
typedef size_t   (*print_t)(   rl_data_t stream, rl_data_t value );

// table & array methods
typedef rl_data_t (*aref_t)(   rl_data_t array, rl_data_t key );
typedef rl_data_t (*xef_t)(    rl_data_t array, rl_data_t key, rl_data_t val );
typedef rl_data_t (*put_t)( rl_data_t coll, rl_data_t key );
typedef rl_data_t (*pop_t)( rl_data_t coll, rl_data_t key );



#endif
