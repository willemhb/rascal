#ifndef rascal_string_h
#define rascal_string_h

#include "core.h"
#include "ctypes.h"
#include "strutils.h"
#include "describe/array.h"
#include "describe/safety.h"

struct string_t ArrayType(char_t, encoding, 1);

#define asstr(x)  ((string_t*)ptr(x))
#define slen(x)   (asstr(x)->len)
#define scap(x)   (asstr(x)->cap)
#define schars(x) (asstr(x)->chars)

DeclareSafeCast(string, string_t*);
DeclareTypeP(string);
DeclareArraySize(string);
DeclareArrayData(string, character_t);

#define tostring(x)                       GenericSafeCast(string)
#define stringp(x)                        GenericTypeP(string)
#define sizeofstring(x, padded, inlined)  GenericArraySize(string)
#define stringdata(x)                     GenericArrayData(string)

// api ------------------------------------------------------------------------
value_t string(char_t *chars );
value_t glstring(char_t *chars );
size_t  stringsize( object_t *string );

char_t  sref( object_t *xs, index_t i );
index_t sset( object_t *xs, index_t i, char_t ch );
arity_t sput( object_t **xs, char_t ch );
arity_t sputn( object_t **xs, arity_t nchrs, ... );

#endif
