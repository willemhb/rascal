#ifndef rascal_object_h
#define rascal_object_h

#include "common.h"
#include "memory.h"

/* header for any file implementing a rascal type. */
// internal values ------------------------------------------------------------
// internal macros ------------------------------------------------------------
#define getf(type, x, field)            (as_##type(x)->field)
#define getf_s(type, x, field, fname)   (to_##type(x, fname)->field)

// C types --------------------------------------------------------------------
struct closure_t
{
  value_t name;
  value_t doc;
  value_t form;

  value_t envt;
  value_t vals;
  value_t code;
};

struct symbol_t
{
  uint_t   length;
  ushort_t flags   : 13;
  ushort_t keyword :  1;
  ushort_t gensym  :  1;
  ushort_t bound   :  1;
  ushort_t tag;

  value_t  bind;
  value_t  constant;
  value_t  doc;

  idno_t   idno;
  hash_t   hash;
  char name[0];
};

struct cons_t
{
  value_t car;
  value_t cdr;
};

struct binary_t
{
  uint_t   length;
  uchar_t  Ctype;
  uchar_t  flags  :  7;
  uchar_t  string :  1; // space allocated for \nul?
  ushort_t tag;

  char_t   space[0];
};

struct tuple_t
{
  uint_t   length;
  ushort_t flags; // unused
  ushort_t tag;

  value_t  space[0];
};

typedef struct node_t
{
  value_t left, right;
  value_t key;
  value_t bind;
} node_t;

struct map_t
{
  uint_t   length;
  ushort_t flags;
  ushort_t tag;

  value_t  data;
};

// utilities ------------------------------------------------------------------
// value api ------------------------------------------------------------------
size_t   lisp_size(value_t x );
type_t   lisp_type( value_t x );
order_t  lisp_order( value_t x, value_t y );
size_t   length( value_t x ); // generic length

// environment api ------------------------------------------------------------
value_t lookup( value_t e, value_t n );
value_t extend( value_t e, value_t n );
value_t assign( value_t e, value_t n, value_t b );
value_t capture( value_t e );

// constructors ---------------------------------------------------------------
value_t flonum( double d );
value_t boolean( int b );
value_t symbol( char *name, bool interned );
value_t cons( value_t ca, value_t cd );
value_t consn( value_t *v, size_t n );
value_t listn( value_t *v, size_t n );
value_t tuple( value_t *v, size_t n );
value_t map( value_t *v, size_t n );
value_t string( char *c, size_t n );
value_t bytecode( ushort *b, size_t n );
value_t closure( value_t n, value_t d, value_t e, value_t v, value_t c );

// global constructors --------------------------------------------------------
value_t gl_string( char *c, size_t n );

// accessors -----------------------------------------------------------------
value_t tup_ref( value_t t, size_t n );
value_t tup_set( value_t t, size_t n, value_t x );
value_t tup_put( value_t t, value_t x );
value_t tup_pop( value_t t, size_t n );

value_t map_ref( value_t m, value_t k );
value_t map_set( value_t m, value_t k, value_t b );
value_t map_put( value_t m, value_t k, value_t b );
value_t map_pop( value_t m, value_t k );

sint_t  str_ref( value_t s, size_t n );

// macros ---------------------------------------------------------------------
// unboxing macros ------------------------------------------------------------
#define as_cons(x)     as_type(cons_t*, pval, x)
#define as_symbol(x)   as_type(symbol_t*, pval, x)
#define as_tuple(x)    as_type(tuple_t*, pval, x)
#define as_map(x)      as_type(map_t*, pval, x)
#define as_node(x)     as_type(node_t*, pval, x)
#define as_binary(x)   as_type(binary_t*, pval, x)
#define as_string(x)   as_type(string_t*, pval, x)
#define as_bytecode(x) as_type(bytecode_t*, pval, x)
#define as_closure(x)  as_type(closure_t*, pval, x)

#define as_flonum(x)   as_type(flonum_t, fval, x)
#define as_integer(x)  as_type(integer_t, )
#define as_port(x)     as_type(port_t, ival, x)
#define as_boolean(x)  as_type(boolean_t, ival, x)

// accessor macros ------------------------------------------------------------
#define car(x) getf(cons, x, car)
#define cdr(x) getf(cons, x, cdr)
#define cddr(x) cdr( cdr( x ) )
#define cdar(x) cdr( car( x ) )
#define cadr(x) car( cdr( x ) )
#define caar(x) car( car( x ) )
#define caddr(x) car( cddr( x ) )
#define cdddr(x) cdr( cddr( x ) )
#define cadddr(x) car( cdddr( x ) )

#define sym_tag(x) getf(symbol, x, tag)
#define tup_tag(x) getf(tuple, x, tag)
#define clo_tag(x) getf(closure, x, tag)
#define bin_tag(x) getf(string, x, tag)
#define map_tag(x) getf(map, x, tag)

#define sym_length(x)   getf(symbol, x, length)
#define sym_bind(x)     getf(symbol, x, bind)
#define sym_idno(x)     getf(symbol, x, idno)
#define sym_gensym(x)   getf(symbol, x, gensym)
#define sym_keyword(x)  getf(symbol, x, keyword)
#define sym_constant(x) getf(symbol, x, constant)
#define sym_bound(x)    getf(symbol, x, bound)
#define sym_hash(x)     getf(symbol, x, hash)
#define sym_name(x)     getf(symbol, x, name)

#define tup_length(x) getf(tuple, x, length)
#define tup_space(x) getf(tuple, x, space)

#define map_length(x) getf(map, x, length)
#define map_data(x) getf(map, x, data)
#define node_left(x) getf(node, x, left)
#define node_right(x) getf(node, x, right)
#define node_key(x) getf(node, x, key)
#define node_bind(x) getf(node, x, bind)

#define bin_length(x) getf(binary, x, length)
#define bin_Ctype(x)  getf(binary, x, Ctype)
#define bin_string(x) getf(binary, x, string)
#define bin_space(x) getf(binary, x, space)

#define clo_name(x) getf(closure, x, name)
#define clo_doc(x) getf(closure, x, doc)
#define clo_form(x) getf(closure, x, form)
#define clo_envt(x) getf(closure, x, envt)
#define clo_vals(x) getf(closure, x, vals)
#define clo_code(x) getf(closure, x, vals)

// safe accessor macros -------------------------------------------------------
#define car_s(x, fn) getf_s(cons, x, car, fn)
#define cdr_s(x, fn) getf_s(cons, x, cdr, fn)

#define sym_bind_s(x, fn) getf_s(symbol, x, bind, fn)
#define sym_name_s(x, fn) getf_s(symbol, x, name, fn)

#endif
