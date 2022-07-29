#ifndef rascal_table_h
#define rascal_table_h

#include "rascal.h"

// utilities ------------------------------------------------------------------
value_t  table_assoc( value_t xt, value_t k );
value_t  dict_put( value_t *xd, value_t k );
value_t  set_put( value_t *xs, value_t k );

// predicates -----------------------------------------------------------------
bool is_table(value_t x);
bool is_dict(value_t x);
bool is_set(value_t x);

// constructors ---------------------------------------------------------------
value_t table( type_t xt, size_t n, value_t *kb );

value_t dict( size_t n, value_t *kb );
index_t dict_s(size_t n, value_t *kb );

value_t set( size_t n, value_t *v );
index_t set_s(size_t n, value_t *kb );

// methods --------------------------------------------------------------------
int    table_order( value_t x, value_t y );
hash_t set_hash( value_t x );
hash_t dict_hash( value_t x );
size_t set_prin( FILE *ios, value_t x );
size_t dict_prin( FILE *ios, value_t x );

// builtins -------------------------------------------------------------------
void r_builtin(dict);
void r_builtin(is_dict);

void r_builtin(set);
void r_builtin(is_set);

void r_builtin(is_table);
void r_builtin(assoc);

// initialization -------------------------------------------------------------
void table_init( void );

// utility macros -------------------------------------------------------------
#define popcnt(x) __builtin_popcount(x)
#define ffs(x)    __builtin_ffs(x)

#define astable(x)     ((table_t*)pval(x))
#define tdepth(x)      get(table, x, depth)
#define tsize(x)       get(table, x, size)
#define tbmap(x)       get(table, x, bmap)
#define tchildren(x)   get(table, x, children)
#define tdata(x)       get(table, x, data)
#define tcache(x)      get(table, x, cache)

#define init_table(o, t, h, k, b)					\
  do									\
    {									\
      init_ob(o, t, C_sint64, 0, false, sizeof(table_t));		\
      ((table_t*)o)->hash  = h;						\
      ((table_t*)o)->key   = k;						\
      ((table_t*)o)->bind  = b;						\
      ((table_t*)o)->left  = (t<<24)|tag_immediate;			\
      ((table_t*)o)->right = (t<<24)|tag_immediate;			\
    } while (0)

#define init_dict(o, h, k, b)			\
  init_table(o, type_dict, h, k, b)

#define init_set(o, h, k)			\
  init_table(o, type_set, h, k, k)

#endif
