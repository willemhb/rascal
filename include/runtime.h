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

/* Globals */
/* Global state objects */
extern State Vm;         // main Vm state object

/* Standard streams */
extern Port StdIn;
extern Port StdOut;
extern Port StdErr;

/* Empty singletons */
extern List EmptyList;
extern Vec  EmptyVec;
extern Map  EmptyMap;

#endif
