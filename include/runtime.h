#ifndef rl_runtime_h
#define rl_runtime_h

#include "values.h"

/* Just some preliminary typedefs and globals for runtime state. */
// internal structure types (not accessible in Rascal)
typedef struct State  State;
typedef struct HFrame HFrame;
typedef struct EFrame EFrame;

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
extern State Vm;

#endif
