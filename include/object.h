#ifndef rascal_object_h
#define rascal_object_h

#include "rascal.h"

/* constructors, initializers, api callbacks, and utilities for
   working with objects and values */

// constructors ---------------------------------------------------------------
value_t symbol( char *name );
value_t gensym( char *name );

index_t gensym_s( char *name );

value_t builtin( char *name, void (*callback)( size_t n ) );
value_t closure( value_t envt, value_t vals, value_t code );

index_t closure_s( value_t *envt, value_t *vals, value_t *code );

value_t cons( value_t car, value_t cdr );
index_t list_s( size_t n, value_t *args );
index_t consn_s( size_t n, value_t *args );
index_t cons_s( value_t *car, value_t *cdr );

size_t  cons_length( value_t x );

value_t vector( size_t n, value_t *args );
index_t vector_s( size_t n, value_t *args );
value_t resize_vector( value_t vec, size_t n );

value_t binary( size_t n, Ctype_t ctype, void *args );
index_t binary_s( size_t n, Ctype_t ctype, void *args );
value_t resize_binary( value_t v, size_t n );

value_t fixnum( fixnum_t x );
value_t boolean( int x );

// predicates -----------------------------------------------------------------
bool is_symbol( value_t x );
bool is_gensym( value_t x );
bool is_keyword( value_t x );
bool is_bound( value_t x );

bool is_cons( value_t x );
bool is_nil( value_t x );
bool is_list( value_t x );

bool is_function( value_t x );
bool is_builtin( value_t x );
bool is_closure( value_t x );

bool is_vector( value_t x );
bool is_binary( value_t x );

bool is_fixnum( value_t x );
bool is_boolean( value_t x );
bool is_true( value_t x );
bool is_false( value_t x );

// safecasts ------------------------------------------------------------------
cons_t    *tocons( const char *fname, value_t x );
symbol_t  *tosymbol( const char *fname, value_t x );
closure_t *toclosure( const char *fname, value_t x );
builtin_t *tobuiltin( const char *fname, value_t x );
vector_t  *tovector( const char *fname, value_t x );
binary_t  *tobinary( const char *fname, value_t x );
fixnum_t   tofixnum( const char *fname, value_t x );
boolean_t  toboolean( const char *fname, value_t x );

// object model API -----------------------------------------------------------
type_t r_type( value_t x );
size_t r_size( value_t x );
int    r_order( value_t x, value_t y );
hash_t r_hash( value_t x, value_t y );

// convenience macros ---------------------------------------------------------
#define asobject(x)  ((object_t*)pval(x))
#define ascons(x)    ((cons_t*)pval(x))
#define asvector(x)  ((vector_t*)pval(x))
#define asbinary(x)  ((binary_t*)pval(x))
#define assymbol(x)  ((symbol_t*)pval(x))
#define asbuiltin(x) ((builtin_t*)pval(x))
#define asclosure(x) ((closure_t*)pval(x))

#define obhead(x)    (asobject(x)->base)
#define obspace(x)   (&(asobject(x)->space[0]))

#define asize(x)   (((size_t*)pval(x))[1])
#define adata(x)   (((void**)pval(x))[2])
#define alength(x) (((size_t*)pval(x))[3])

#define car(x) (ascons(x)->car)
#define cdr(x) (ascons(x)->cdr)

#define cddr(x) car(cdr(x))
#define cdar(x) cdr(car(x))
#define cadr(x) car(cdr(x))
#define caar(x) car(car(x))

#define caadr(x) car(car(cdr(x)))

#define car_s(f, x) (tocons(f,x)->car)
#define cdr_s(f, x) (tocons(f,x)->cdr)

#define init_ob(o, t, C, e, a, b)		\
  do						\
    {						\
      ((header_t*)o)->type      = t;		\
      ((header_t*)o)->Ctype     = C;		\
      ((header_t*)o)->encoding  = e;		\
      ((header_t*)o)->is_array  = a;		\
      ((header_t*)o)->base_size	= b;		\
    } while (0)

#define init_symbol(o, h, n, l)						\
  do {									\
    init_ob(o,								\
	    tag_symbol,							\
	    C_uint8,							\
	    enc_utf8,							\
	    false,							\
	    sizeof(symbol_t)+l+1);					\
    									\
    ((symbol_t*)o)->idno = Symcnt++;					\
    ((symbol_t*)o)->bind = val_unbound;					\
    ((symbol_t*)o)->hash = h;						\
  } while (0)

#define init_builtin(o, s, c)						\
  do									\
    {									\
      init_ob(o, tag_builtin, C_sint64, 0, false, sizeof(builtin_t));	\
      ((builtin_t*)o)->callback = c;					\
      assymbol(s)->bind = tagp(o, tag_boxed );				\
    } while (0)


#define init_array(o, t, C, e, l, s)			\
  do {							\
    init_ob( o, t, C, e, true, sizeof(vector_t));	\
    asize( o )   = s;					\
    adata( o )   = (uchar*)o + sizeof(vector_t);	\
    alength( o ) = l;					\
  } while (0)

#define init_vector(o, l, s)			\
  init_array(o, tag_vector, C_sint64, 0, l, s)

#define init_binary(o, C, l, s)			\
  init_array(o, tag_binary, C, 0, l, s)

#endif
