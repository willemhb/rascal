#ifndef rascal_array_h
#define rascal_array_h

#include "object.h"
#include "list.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    C_sint8, C_uint8, C_ascii, C_latin1, C_utf8,

    C_sint16, C_uint16, C_utf16,

    C_sint32, C_uint32, C_utf32, C_float32,

    C_sint64, C_uint64, C_pointer, C_float64
  } Ctype_t;

typedef enum
  {
   vector_fl_envt    =1,
   vector_fl_stack   =2
  } vector_fl_t;

typedef enum
  {
    table_fl_symbol_table  = 1,
    table_fl_cycle_detector= 2
  } table_fl_t;

struct binary_t
{
  HEADER;
  uint len, cap;
  void *data;
};

struct vector_t
{
  HEADER;
  uint len, cap;
  value_t *data;
};

typedef struct table_t
{
  HEADER;
  uint cnt, cap;
  
  object_t **entries;
} table_t;

typedef struct // a pair with a hash hidden after the cdr
{
  cons_t base;
  ulong  hash;
} entry_t;

typedef struct vector_t stack_t;

// forward declarations -------------------------------------------------------
size_t    Ctype_size(Ctype_t C);
bool      Ctype_is_encoded(Ctype_t C);
bool      Ctype_is_integer(Ctype_t C);

value_t   vector( value_t *args, size_t n_args );
vector_t *new_vector( size_t n_args, size_t *padded, bool gl );
void      init_vector( vector_t *v, uint fl, size_t n, size_t p, value_t *a );
value_t   vector_ref( vector_t *xb, uint n);
value_t   vector_set( vector_t *xb, uint n, value_t v );
value_t   vector_conj( vector_t *xb, uint n, value_t v );
value_t   vector_pop( vector_t *xb, uint n );

void      resize_stack( stack_t *s );
void      stack_push( stack_t *s, value_t v );
value_t   stack_pop( stack_t *s );
void      stack_dup( stack_t *s );

value_t   binary( void *args, size_t n_args, Ctype_t Ctype );
binary_t *new_binary( size_t n_args, size_t *padded, bool gl, Ctype_t Ctype );
value_t   binary_ref( binary_t *xb, uint n );
value_t   binary_set( binary_t *xb, uint n, void *v );
value_t   binary_conj( binary_t *xb, uint n, void *v );
value_t   binary_pop( binary_t *xb, uint n );

value_t   table(value_t *args, size_t n_args);
value_t   table_ref(table_t *xt, value_t key);
value_t   table_set(table_t *xt, value_t key, value_t bind);
value_t   table_put(table_t *xt, value_t key, value_t bind);
value_t   table_pop(table_t *xt, value_t key);

// macros & statics -----------------------------------------------------------
#define as_binary(x)     asa(binary_t*, x, pval)
#define as_vector(x)     asa(vector_t*, x, pval)
#define as_table(x)      asa(table_t*, x, pval)
#define as_entry(x)      asa(entry_t*, x, pval)

#define bin_data(x)      getf(binary_t*, x, data)
#define bin_len(x)       getf(binary_t*, x, len)
#define bin_cap(x)       getf(binary_t*, x, cap)

#define vec_data(x)      getf(vector_t*, x, data)
#define vec_len(x)       getf(vector_t*, x, len)
#define vec_cap(x)       getf(vector_t*, x, cap)

#define table_entries(x) getf(table_t*, x, entries)
#define table_cnt(x)     getf(table_t*, x, cnt)
#define table_cap(x)     getf(table_t*, x, cap)

#define entry_hash(x)    getf(entry_t*, x, hash)

static inline tag_p(table, TABLE)
static inline tag_p(vector, VECTOR)
static inline tag_p(binary, BINARY)
static inline ob_flag_p(stack, VECTOR, vector_fl_stack)
static inline ob_flag_p(envt, VECTOR, vector_fl_envt)

#endif
