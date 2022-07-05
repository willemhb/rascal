#ifndef rascal_value_h
#define rascal_value_h

#include "core.h"

// api ------------------------------------------------------------------------
bool_t  rtoCbool(value_t xv);
type_t  rtypeof(value_t xv);
size_t  rsizeof(value_t xv);
hash_t  rhash(value_t xv);
char_t *rtypename(value_t xv);

#endif
