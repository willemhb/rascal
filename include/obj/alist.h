#ifndef rascal_alist_h
#define rascal_alist_h

#include "obj.h"
#include "template/array.h"

// similar to a stack, but with a different reallocation algorithm
DECL_OBJ(alist);

ARRAYLIST(alist, val_t);

#endif
