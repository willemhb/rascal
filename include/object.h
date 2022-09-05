#ifndef rascal_object_h
#define rascal_object_h

#include "common.h"
#include "memory.h"

// C types --------------------------------------------------------------------
struct function_t
{
  /* namespace info */
  value_t names;
  value_t binds;

  /* metadata */
  value_t name;
  value_t props;

  /* execution info */
  value_t vals;
  value_t code;

  /* arity & flags */
  uint n_args;
  uint n_vars;
  uint n_stack;

  bool is_vargs;
};

struct symbol_t
{
  value_t  bind;
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

struct string_t
{
  value_t length;
  char space[0];
};

struct tuple_t
{
  value_t  length;
  value_t  space[0];
};

typedef struct entry_t
{
  value_t key; // integer (terminal), list (collision), or map (child)
  hash_t hash;
} entry_t;

typedef struct node_t
{
  value_t cache;
  uint    bitmap, depth;
  entry_t entries[0];
} node_t;

struct table_t
{
  value_t length;
  value_t compare;
  value_t mapping; // HAMT of hashed entries
  value_t values;  // vector of key/value pairs
};

// Macros macros ------------------------------------------------------------
#define getf(type, x, field)            (as_##type(x)->field)
#define getf_s(type, x, field, fname)   (to_##type(x, fname)->field)

// unboxing macros ------------------------------------------------------------
#define as_cons(x)     as_type(cons_t*, pval, x)
#define as_symbol(x)   as_type(symbol_t*, pval, x)
#define as_vector(x)   as_type(vector_t*, pval, x)
#define as_table(x)    as_type(table_t*, pval, x)
#define as_function(x) as_type(function_t*, pval, x)
#define as_string(x)   as_type(string_t*, pval, x)
#define as_port(x)     as_type(port_t*, pval, x)

#define as_character(x) as_type(character_t, lval, x)
#define as_flonum(x)   as_type(flonum_t, fval, x)
#define as_integer(x)  as_type(integer_t, lval, x)
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

#endif
