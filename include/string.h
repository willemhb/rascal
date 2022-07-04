#ifndef rascal_string_h
#define rascal_string_h

#include "core.h"

struct string_t {
  object_t base;
  size_t len, cap;
  char_t *chars;
};

#define asstr(x)  ((string_t*)ptr(x))
#define slen(x)   (asstr(x)->len)
#define scap(x)   (asstr(x)->cap)
#define schars(x) (asstr(x)->chars)

// api ------------------------------------------------------------------------
char_t  sref( object_t *xs, index_t i );
index_t sset( object_t *xs, index_t i, char_t ch );
arity_t sput( object_t **xs, char_t ch );
arity_t sputn( object_t **xs, arity_t nchrs, ...);

#endif
