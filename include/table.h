#ifndef rascal_table_h
#define rascal_table_h

#include "core.h"

struct table_t {
  object_t base;
  value_t len, entry;

  ulong_t hash;

  object_t *left, *right;
};

#define astab(x)    ((table_t*)ptr(x))
#define tablen(x)   (astab(x)->len)
#define tabentry(x) (astab(x)->entry)
#define tabhash(x)  (astab(x)->hash)
#define tableft(x)  (astab(x)->left)
#define tabright(x) (astab(x)->right)

// api ------------------------------------------------------------------------
bool_t  locate(object_t *tab, value_t key, object_t **buf);
bool_t  locates(object_t *tab, char_t *key, object_t **buf);

value_t table(value_t *args, size_t nargs, bool_t global);

#endif
