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
typedef Val    (*BuiltinFn)(Proc* p, size32 n);       // handler for native primfn

// lifetime methods
typedef void   (*TraceFn)(State* vm, void* x);        // trace lifetime method
typedef void   (*FreeFn)(State* vm, void* x);         // free lifetime method
typedef void*  (*CloneFn)(State* vm, void* x);        // clone lifetime method
typedef void   (*SealFn)(State* vm, void* x);         // seal lifetime method

// comparison interface methods
typedef hash64 (*HashFn)(Val x);                      // hash method
typedef bool   (*EgalFn)(Val x, Val y);               // egal method
typedef int    (*OrderFn)(Val x, Val y);              // order method

// sequence interface methods
typedef void*  (*SeqFn)(void* x);                     // return a sequence
typedef bool   (*DoneFn)(void* x);                    // 
typedef Val    (*FirstFn)(void* x);                   // get the first item in sequence
typedef void*  (*RestFn)(void* x);                    // get the reset of the sequence

// miscellaneous interface methods
typedef size64 (*PrintFn)(State* vm, Port* p, Val x); // print method

/* Globals */
/* Global state objects */
extern State Vm;
extern Proc  Main;

#endif
