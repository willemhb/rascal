#ifndef rascal_alist_h
#define rascal_alist_h

#include "obj.h"
#include "template/alist.h"

// similar to a stack, but with a different reallocation algorithm
DECL_OBJ(alist);

DECL_ALIST(alist, val_t);

#endif
