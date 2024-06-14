#include "val/table.h"
#include "val/type.h"

/* Internal forward declarations */


/* Globals */
/* HAMT parameters */
#define OMAP_LIMIT  16
#define LEVEL_SHIFT  6
#define MAX_SHIFT    8
#define LEVEL_SIZE  64
#define LEVEL_MASK  63

/* Internal APIs */

/* External APIs */

/* Mutable tables */
#define MUTABLE_TABLE(T, E, K, V, t, ...)       \

MUTABLE_TABLE(MMap, MMEntry, Val, Val, mmap);
MUTABLE_TABLE(SCache, SCEntry, char*, Str*, scache);
MUTABLE_TABLE(EnvMap, EMEntry, Sym*, Ref*, emap, int scope);

#undef MUTABLE_TABLE
#undef OMAP_LIMIT
#undef LEVEL_SHIFT
#undef MAX_SHIFT
#undef LEVEL_SIZE
#undef LEVEL_MASK
