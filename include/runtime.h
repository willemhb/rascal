#ifndef rl_runtime_h
#define rl_runtime_h

#include "labels.h"
#include "values.h"

/* Just some preliminary typedefs and globals for runtime state. */
// internal structure types (not immediately accessible in Rascal)
typedef struct State    State;    // all of Rascal's global state lives here
typedef struct Proc     Proc;     // running state (IP, SP, etc) lives here
typedef struct HFrame   HFrame;   // keeps track of  values in the C stack that need to be garbage collected
typedef struct EFrame   EFrame;   // error frame
typedef struct StrTable StrTable; // for string interning
typedef struct VTable   VTable;   // internal methods for values of a given type

// internal function pointer types
typedef Val    (*BuiltinFn)(Proc* p, size64 n);
typedef int    (*RascalFn)(State* vm);
typedef void   (*TraceFn)(State* vm, void* x);
typedef void   (*FreeFn)(State* vm, void* x);
typedef void   (*SealFn)(State* vm, void* x);
typedef hash64 (*HashFn)(Val x);
typedef bool   (*EgalFn)(Val x, Val y);
typedef int    (*OrderFn)(Val x, Val y);

/* Globals */
/* Global state objects */
extern State Vm;
extern Proc  Main;

#endif
