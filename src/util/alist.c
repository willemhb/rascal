#include "util/alist.h"
#include "util/tpl/impl/alist.h"

/* commentary */

/* C types */

/* globals */

/* API */
MAKE_ALIST(alist, void*);
RESIZE_ALIST(alist, void*);
FREE_ALIST(alist, void*);
ALIST_PUSH(alist, void*);
ALIST_POP(alist, void*);
ALIST_PUSHN(alist, void*, void*);
ALIST_POPN(alist, void*);
ALIST_REF(alist, void*);
ALIST_SET(alist, void*);
ALIST_SWAP(alist, void*);

/* runtime */

/* convenience */
