#ifndef rl_runtime_h
#define rl_runtime_h

#include "values.h"

/* Just some preliminary typedefs and globals for runtime state. */
/* Frame types */
typedef struct HFrame HFrame;

// internal function pointer types
typedef void   (*TraceFn)(void* x);
typedef void   (*FreeFn)(void* x);
typedef void   (*CloneFn)(void* x);
typedef hash_t (*HashFn)(Val x);
typedef bool   (*EgalFn)(Val x, Val y);
typedef int    (*OrderFn)(Val x, Val y);
typedef size_t (*SizeofFn)(void* x);
typedef void   (*CastFn)(Val x, void* s);

/* Globals */
/* Global state objects */
extern State Vm;         // main Vm state object
extern Proc  Main;       // main process (for the time being this is the only process)

/* Standard streams */
extern Port StdIn;
extern Port StdOut;
extern Port StdErr;

/* Empty singletons */
extern List EmptyList;
extern Vec  EmptyVec;
extern Map  EmptyMap;

/* Global environment state */
extern Env    Globals;
extern NSMap  NameSpaces;
extern SCache Strings;

#endif
