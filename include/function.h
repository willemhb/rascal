#ifndef rascal_function_h
#define rascal_function_h

#include "rascal.h"

// constructors ---------------------------------------------------------------
value_t builtin( char *name, void (*callback)( size_t n ) );
value_t closure( value_t envt, value_t vals, value_t code );

index_t closure_s( value_t *envt, value_t *vals, value_t *code );

// predicates -----------------------------------------------------------------
bool is_function( value_t x );
bool is_builtin( value_t x );
bool is_closure( value_t x );

// safecasts ------------------------------------------------------------------
closure_t  *toclosure( const char *fname, value_t x );
builtin_t  *tobuiltin( const char *fname, value_t x );

// methods --------------------------------------------------------------------
int    closure_order( value_t x, value_t y );
hash_t closure_hash( value_t x );

// builtins -------------------------------------------------------------------
void builtin_is_closure( size_t n );
void builtin_is_builtin( size_t n );
void builtin_is_function( size_t n );

// initialization -------------------------------------------------------------
void function_init( void );

// convenience macros ---------------------------------------------------------
#define asbuiltin(x) ((builtin_t*)pval(x))
#define asclosure(x) ((closure_t*)pval(x))

#define clenvt(x) get(closure, x, envt)
#define clvals(x) get(closure, x, vals)
#define clcode(x) get(closure, x, code)

#define init_builtin(o, s, c)						\
  do									\
    {									\
      init_ob(o, tag_builtin, C_sint64, 0, false, sizeof(builtin_t));	\
      ((builtin_t*)o)->callback = c;					\
      assymbol(s)->bind = tagp(o, tag_boxed );				\
    } while (0)

#define init_closure(o, e, v, c)					\
  do									\
    {									\
      init_ob(o, tag_closure, C_sint64, 0, false, sizeof(closure_t));	\
      asclosure(o)->envt = e;						\
      asclosure(o)->vals = v;						\
      asclosure(o)->code = c;						\
    } while(0)

#endif
