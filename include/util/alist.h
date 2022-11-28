#ifndef rl_util_alist_h
#define rl_util_alist_h

#include "common.h"
#include "util/tpl/decl/alist.h"

/* commentary

   generic alist type. */

/* C types */
typedef ALIST(void*) alist_t;

/* API */
MAKE_ALIST(alist, void*);
RESIZE_ALIST(alist, void*);
FREE_ALIST(alist, void*);
ALIST_PUSH(alist, void*);
ALIST_POP(alist, void*);
ALIST_PUSHN(alist, void*);
ALIST_POPN(alist, void*);
ALIST_REF(alist, void*);
ALIST_SET(alist, void*);
ALIST_SWAP(alist, void*);

/* runtime */

/* convenience */

#endif
