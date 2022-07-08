#ifndef rascal_table_h
#define rascal_table_h

#include "rascal.h"

// table root structure
struct table_t {
  object_t base;
  size_t   offset    : 56; // offset from the
  size_t   rootp     :  1; // table root?
  size_t   sharep    :  1;
  size_t   flags     :  6;

  object_t *keys;
  object_t *values;
};

// table element structure
typedef struct node_t {
  object_t base;
  index_t location : 56;
  index_t rootp    :  1; // table root?
  index_t sharep   :  1;
  index_t flags    :  6;

  object_t *left, *right;
} node_t;

#define astab(x)     ((table_t*)ptr(x))
#define asnode(x)    ((node_t*)ptr(x))
#define taboffset(x) (astab(x)->offset)
#define tabloc(x)    (asnode(x)->location)
#define tabrootp(x)  (astab(x)->rootp)
#define tabsharep(x) (astab(x)->sharep)
#define tabkeys(x)   (astab(x)->keys)
#define tabvals(x)   (astab(x)->values)
#define tableft(x)   (asnode(x)->left)
#define tabright(x)  (asnode(x)->right)

// api ------------------------------------------------------------------------
bool_t  tabhas(object_t *tab, value_t key);
value_t tabref(object_t *tab, value_t key);
value_t tabset(object_t *tab, value_t key, value_t bind );
value_t tabput(object_t *tab, value_t key, value_t bind );
value_t tabpop(object_t *tab, value_t key);

value_t intern(object_t *t, char_t *k, index_t i, index_t (*constructor)( char_t *k, hash_t h ) );

value_t mk_table( value_t *args, size_t nargs, bool_t global );
value_t mk_dict( value_t *args, size_t nargs, bool_t global );
value_t mk_set( value_t *args, size_t nargs, bool_t global );

#endif
