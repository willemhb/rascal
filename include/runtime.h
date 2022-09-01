#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "common.h"

// describe macros ------------------------------------------------------------
#define ht_pred(t, T)  bool is_##t(value_t x) { return hitag(x) == T; }
#define wt_pred(t, T)  bool is_##t(value_t x) { return widetag(x) == T; }
#define ot_pred(t, T)  bool is_##t(value_t x) { return hitag(x) == OBJECT && widetag(car(x)) == T; }
#define val_pred(v, V) bool is_##v(value_t x) { return x == V; }
#define type_pred(t, T) bool is_##t(value_t x) { return lisp_type(x) == T; }

#define const_pred(t, v) bool is_##t(value_t x) { (void)x; return v; }

#define make_safecast(ctype, type, cnvt)	\
  ctype to_##type(value_t x, const char *f)	\
  {						\
    require( is_##type(x),			\
	     x,					\
	     f,					\
	     "expected type %s, got",		\
	     #type);				\
    return (ctype)cnvt(x);			\
  }

#define flag_pred(fname, ctype, type, cnvt, field, value)		\
  bool is_##fname(value_t x)						\
  {									\
    return !is_immediate(x)						\
      && is_##type(x)							\
      && ((ctype)cnvt(x))->field == value;				\
  }

// utilities ------------------------------------------------------------------
// error signaling ------------------------------------------------------------
void    error( value_t a, const char *n, const char *f, ... );
void    verror( value_t a, const char *n, const char *f, va_list va );
void    prin_error( value_t a, const char *n, const char *f, va_list va );

// validation -----------------------------------------------------------------
void    require( bool t, value_t a, const char *n, const char *f, ...);
size_t  argco( size_t g, size_t e, bool v, const char *n );
size_t  argcos( size_t g, const char *n, size_t e, ...);
type_t  argtype( value_t a, const  char *n, type_t e );
type_t  argtypes( value_t a, const char *n, size_t e, ...);

// predicates -----------------------------------------------------------------
// type predicates ------------------------------------------------------------
bool         is_nil( value_t x );

bool         is_boolean( value_t x );
bool         is_integer( value_t x );
bool         is_flonum( value_t x );

bool         is_builtin( value_t x );
bool         is_closure( value_t x );

bool         is_cons( value_t x );

bool         is_tuple( value_t x);
bool         is_map( value_t x );
bool         is_binary( value_t x );

bool         is_symbol( value_t x );

bool         is_port( value_t x );

// other tag predicates -------------------------------------------------------
bool         is_immediate( value_t x );
bool         is_function( value_t x );

// union-type predicates ------------------------------------------------------
bool         is_list( value_t x );

// symbol predicates ----------------------------------------------------------
bool         is_gensym( value_t x );
bool         is_keyword( value_t x );
bool         is_constant( value_t x );
bool         is_bound( value_t x );

// function predicates --------------------------------------------------------
bool         is_vargs( value_t x );
bool         is_constructor( value_t x );

// binary predicates ----------------------------------------------------------
bool         is_ascii(value_t x);
bool         is_latin1(value_t x);
bool         is_utf8(value_t x);
bool         is_utf16(value_t x);
bool         is_utf32(value_t x);

// map predicates -------------------------------------------------------------
bool         is_node( value_t x );

// misc predicates ------------------------------------------------------------
bool         is_true( value_t x );
bool         is_false( value_t x );

// safecasts ------------------------------------------------------------------
boolean_t      to_boolean(value_t x, const char *n);
integer_t      to_integer(value_t x, const char *n);
flonum_t       to_flonum(value_t x, const char *n);
port_t         to_port(value_t x, const char *n);

builtin_t      to_builtin(value_t x, const char *n);
closure_t     *to_closure(value_t x, const char *n); 

cons_t        *to_cons(value_t x, const char *n);

tuple_t       *to_vector(value_t x, const char *n);
map_t         *to_map(value_t x, const char *n);
binary_t      *to_binary(value_t x, const char *n);

symbol_t      *to_symbol(value_t x, const char *n);

#endif
