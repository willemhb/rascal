#ifndef rascal_str_h
#define rascal_str_h

#include "obj.h"
#include "mem.h"
#include "utils/str.h"
#include "template/str.h"

typedef struct str_t str_t;

DECL_OBJ(str);
DECL_OBJ_API(str);
DECL_STRING(str, char_t);

// globals
extern repr_t *StrRepr;

// convenience

#endif
