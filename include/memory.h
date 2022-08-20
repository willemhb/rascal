#ifndef rascal_memory_h
#define rascal_memory_h

#include <assert.h>

#include "memutils.h"

#include "common.h"

// internal values -------------------------------------------------------------
#define N_VECTOR  32
#define N_STACK   4096
#define N_HEAP    8192
#define RESIZE_F  0.625
#define GROW_F    2.0

// C types --------------------------------------------------------------------
struct closure_t
{
  uint_t   arity;
  uchar_t  type;
  uchar_t  vargs    : 1;
  uchar_t  macro    : 1;
  uchar_t  compiled : 1;
  uchar_t  oargs    : 1;
  uchar_t           : 4;
  ushort_t tag;

  value_t name;
  value_t head;
  value_t envt;
  value_t tplt;
};

struct symbol_t
{
  uint_t   length;
  uchar_t  type;
  uchar_t  gensym   : 1;
  uchar_t  keyword  : 1;
  uchar_t  constant : 1;
  uchar_t  bound    : 1;
  uchar_t           : 4;
  ushort_t tag;

  value_t bind;
  idno_t  idno;
  hash_t  hash;
  char name[0];
};

struct cons_t
{
  value_t car;
  value_t cdr;
};

struct environment_t
{
  uint_t   length;
  uchar_t  type;
  uchar_t  captured : 1;
  uchar_t           : 7;
  ushort_t tag;

  value_t names;
  value_t binds;
  value_t parent;
};

struct binary_t
{
  uint_t   length;
  uchar_t  type;
  uchar_t  Ctype;
  ushort_t tag;

  char_t   space[0];
};

struct vector_t
{
  uint_t   length;
  uchar_t  type;
  uchar_t  height;
  ushort_t tag;

  union
  {
    value_t data; // pointer to actual table nodes (table root)
    
    struct
    {            // table node
      value_t cache;
      value_t space[0];
    };
  };
};

struct stack_t
{
  uint_t sp, fp, cap, cap_min;
  value_t *data;
};

struct heap_t
{
  size_t used, available, alignment;

  bool_t grow, grew, collecting;

  uchar_t *space, *swap;
};

struct gc_frame_t
{
  size_t      length;
  gc_frame_t *next;
  value_t    **saved;
};

typedef struct symbol_node_t
{
  struct symbol_node_t *left, *right;
  hash_t hash;
  char  *name;
  /* node hidden before the interned symbol itself */
  symbol_t entry;
} symbol_node_t;

struct symbol_table_t
{
  size_t num_symbols;
  
  symbol_node_t *root;
};

// implementations ------------------------------------------------------------
void    init_heap(heap_t *h);
void    finalize_heap(heap_t *h);

void    init_stack(stack_t *s);
void    finalize_stack(stack_t *s);

void    init_symbol_table( symbol_table_t *t );
void    finalize_symbol_table( symbol_table_t *t );

// utilities ------------------------------------------------------------------
// misc -----------------------------------------------------------------------
size_t   Ctype_size(Ctype_t ct);
size_t   lisp_size(value_t x);
type_t   lisp_type( value_t x);
Ctype_t  type_Ctype(type_t ct);

// stack manipulation ---------------------------------------------------------
index_t  push( value_t x );
value_t  pop( void );
void     dup( void );
index_t  pushn( size_t n );
value_t  popn( size_t n );

// predicates -----------------------------------------------------------------
bool_t   is_type(value_t x);
bool_t   is_nil(value_t x);

bool_t   is_integer(value_t x);
bool_t   is_boolean(value_t x);
bool_t   is_flonum(value_t x);
bool_t   is_character(value_t x);

bool_t   is_symbol(value_t x);
bool_t   is_gensym(value_t x);
bool_t   is_keyword(value_t x);

bool_t   is_pair(value_t x);
bool_t   is_cons(value_t x);
bool_t   is_list(value_t x);
bool_t   is_cell(value_t x);

bool_t   is_port(value_t x);
bool_t   is_function(value_t x);
bool_t   is_builtin(value_t x);
bool_t   is_environment(value_t x);
bool_t   is_closure(value_t x);
bool_t   is_vector(value_t x);
bool_t   is_immedate(value_t x);

// memory management ----------------------------------------------------------
void    *allocate( size_t n, bool_t global );
void     trace_stack( stack_t *s );
void     trace_symbol_table( symbol_table_t *s );
void     trace_gc_frames( gc_frame_t *f );
void     gc_frame_cleanup( gc_frame_t *f );

// objects --------------------------------------------------------------------
value_t symbol( char *name, bool_t interned );
value_t cons( value_t ca, value_t cd );
value_t vector( value_t *vals, size_t n_vals );
value_t string( char *chrs, Ctype_t encoding, size_t n_vals );
value_t binary( uchar *bytes, Ctype_t Ctype, size_t n_elements );
value_t closure( value_t head, value_t args, value_t body, value_t envt );

// macros ---------------------------------------------------------------------
#define as_cons(x)        as_type(cons_t*, pval, x)
#define as_symbol(x)      as_type(symbol_t*, pval, x)
#define as_vector(x)      as_type(vector_t*, pval, x)
#define as_string(x)      as_type(string_t*, pval, x)
#define as_binary(x)      as_type(binary_t*, pval, x)
#define as_closure(x)     as_type(closure_t*, pval, x)
#define as_environment(x) as_type(environment_t*, pval, x)
#define as_integer(x)     as_type(integer_t, ival, x)
#define as_character(x)   as_type(character_t, ival, x)
#define as_port(x)        as_type(port_t, ival, x)
#define as_boolean(x)     as_type(boolean_t, ival, x)

#define car(x)          getf(cons, x, car)
#define cdr(x)          getf(cons, x, cdr)
#define car_s(x, fn)    getf_s(cons, x, car, fn)
#define cdr_s(x, fn)    getf_s(cons, x, cdr, fn)

#define sym_tag(x)      getf(symbol, x, tag)
#define vec_tag(x)      getf(vector, x, tag)
#define clo_tag(x)      getf(closure, x, tag)
#define str_tag(x)      getf(string, x, tag)
#define bin_tag(x)      getf(binary, x, tag)

#define sym_type(x)      getf(symbol, x, type)
#define vec_type(x)      getf(vector, x, type)
#define clo_type(x)      getf(closure, x, type)
#define str_type(x)      getf(string, x, type)
#define bin_type(x)      getf(binary, x, type)

#define sym_length(x)   getf(symbol, x, length)
#define sym_bind(x)     getf(symbol, x, bind)
#define sym_idno(x)     getf(symbol, x, idno)
#define sym_gensym(x)   getf(symbol, x, gensym)
#define sym_keyword(x)  getf(symbol, x, keyword)
#define sym_constant(x) getf(symbol, x, constant)
#define sym_bound(x)    getf(symbol, x, bound)
#define sym_hash(x)     getf(symbol, x, hash)
#define sym_name(x)     getf(symbol, x, name)

#define vec_length(x)   getf(vector, x, length)
#define vec_height(x)   getf(vector, x, height)
#define vec_data(x)     getf(vector, x, data)
#define vec_cache(x)    getf(vector, x, cache)
#define vec_space(x)    getf(vector, x, space)

#define str_length(x)   getf(string, x, length)
#define str_Ctype(x)    getf(string, x, Ctype)
#define str_space(x)    getf(string, x, space)
#define str_Ctype(x)    getf(string, x, Ctype)

#define bin_length(x)   getf(binary, x, length)
#define bin_Ctype(x)    getf(binary, x, Ctype)
#define bin_space(x)    getf(binary, x, space)

#define env_length(x)   getf(environment, x, length)
#define env_captured(x) getf(environment, x, captured)
#define env_names(x)    getf(environment, x, names)
#define env_binds(x)    getf(environment, x, binds)
#define env_parent(x)   getf(environment, x, parent)

#define clo_arity(x)    getf(closure, x, arity)
#define clo_vargs(x)    getf(closure, x, vargs)
#define clo_macro(x)    getf(closure, x, macro)
#define clo_oargs(x)    getf(closure, x, oargs)
#define clo_args(x)     getf(closure, x, args)
#define clo_envt(x)     getf(closure, x, envt)
#define clo_name(x)     getf(closure, x, name)
#define clo_head(x)     getf(closure, x, head)
#define clo_tplt(x)     getf(closure, x, tplt)

#define RSP_GC_FRAME gc_frame_t __gc_frame__ __attribute__((cleanup(gc_frame_cleanup)))

#define preserve(n, vars...)					\
  value_t *__gc_frame_array__[(n)] = { vars };			\
  RSP_GC_FRAME = { (n), Saved, &__gc_frame_array__[0] };	\
  Saved = &__gc_frame__

#endif
