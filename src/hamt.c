#include <string.h>

#include "hamt.h"
#include "metaobject.h"
#include "object.h"
#include "number.h"

#include "memory.h"

// misc -----------------------------------------------------------------------
// globals --------------------------------------------------------------------
#define SHIFT      0x06ul
#define MAXN       0x40ul
#define MAXS       0x2aul
#define MINN       0x01ul
#define MASK       0x3ful
#define MAXH       0x07ul
#define MAXL       0x08ul
#define KEY(a, o)  ((a)[((o) & MASK) << 1])
#define VAL(a, o)  ((a)[(((o) & MASK) << 1) | 1])

// utilities ------------------------------------------------------------------

// APIs -----------------------------------------------------------------------
// vector ---------------------------------------------------------------------

// dict -----------------------------------------------------------------------

#undef SHIFT
#undef MAXN
#undef MAXS
#undef MINN
#undef MASK
#undef MAXH
#undef MAXL
