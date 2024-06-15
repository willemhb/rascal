#ifndef rl_runtime_h
#define rl_runtime_h

#include "common.h"

/* Just some preliminary typedefs and globals for runtime state. */
typedef struct RState RState;

extern struct List* ClArgs;
extern struct Map*  ClFlags;
extern struct Map*  ClOpts;

#endif
