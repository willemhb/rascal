#ifndef rascal_object_h
#define rascal_object_h

#include "rascal.h"

/* constructors, initializers, api callbacks, and utilities for
   working with objects and values */

// utilities ------------------------------------------------------------------
bool  Cbool( value_t x );
void *pval_s( value_t x );

// predicates -----------------------------------------------------------------
bool is_boxed( value_t x );
bool is_pair( value_t x );
bool is_immediate( value_t x );
bool is_empty( value_t x );

// core -----------------------------------------------------------------------
type_t r_type( value_t x );
size_t r_size( value_t x );
int    r_order( value_t x, value_t y );
bool   r_equals( value_t x, value_t y );
hash_t r_hash( value_t x );

// builtins ------------------------------------------------------------------
void builtin_size( size_t n );
void builtin_order( size_t n );
void builtin_hash( size_t n );
void builtin_is_id( size_t n );
void builtin_is_eq( size_t n );
void builtin_not( size_t n );

// initialization -------------------------------------------------------------
void object_init( void );

// convenience macros ---------------------------------------------------------
#define asobject(x)  ((object_t*)pval(x))

#define ob_head(x)         (asobject(x)->base)
#define ob_type(x)         (ob_head(x).type)
#define ob_Ctype(x)        (ob_head(x).Ctype)
#define ob_encoding(x)     (ob_head(x).encoding)
#define ob_is_array(x)     (ob_head(x).is_array)
#define ob_is_traversed(x) (ob_head(x).is_traversed)
#define ob_base_size(x)    (ob_head(x).base_size)
#define ob_flags(x)        (ob_head(x).flags)

#define ob_space(x)        (&(asobject(x)->space[0]))

#define init_ob(o, t, C, e, a, b)		\
  do						\
    {						\
      ((header_t*)o)->type      = t;		\
      ((header_t*)o)->Ctype     = C;		\
      ((header_t*)o)->encoding  = e;		\
      ((header_t*)o)->is_array  = a;		\
      ((header_t*)o)->base_size	= b;		\
    } while (0)

#endif
