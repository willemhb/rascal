#ifndef rl_runtime_h
#define rl_runtime_h

#include "values.h"

/* Just some preliminary typedefs and globals for runtime state. */
// internal structure types (not immediately accessible in Rascal)
typedef struct State   State;   // all of Rascal's global state lives here
typedef struct Process Process; // 
typedef struct HFrame  HFrame;  // keeps track of  values in the C stack that need to be garbage collected
typedef struct VTable  VTable;  // internal methods for values of a given type

// internal function pointer types
typedef int    (*RascalFn)(State* vm);
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
