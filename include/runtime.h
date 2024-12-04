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
typedef Error  (*BuiltinFn)(Proc* p, size32 n);          // handler for native primfn

// lifetime methods
typedef void   (*TraceFn)(State* vm, void* x);           // trace lifetime method
typedef void   (*FreeFn)(State* vm, void* x);            // free lifetime method
typedef void   (*CloneFn)(State* vm, void* x);           // clone lifetime method (only responsible for duplicating owned data)
typedef void   (*SealFn)(State* vm, void* x, bool d);    // seal lifetime method

// comparison interface methods
typedef hash64 (*HashFn)(Val x);                      // hash method
typedef bool   (*EgalFn)(Val x, Val y);               // egal method
typedef int    (*OrderFn)(Val x, Val y);              // order method

// sequence interface methods
// for types that implement the interface directly
typedef bool   (*EmptyFn)(void* x);                   // empty predicate
typedef Val    (*FirstFn)(void* x);                   // get the first item in sequence
typedef void*  (*RestFn)(void* x);                    // get the reset of the sequence

// for types that use a Seq object
typedef void   (*SInitFn)(Seq* s);
typedef Val    (*SFirstFn)(Seq* s);
typedef void   (*SRestFn)(Seq* s);

// miscellaneous interface methods
typedef size64 (*PrFn)(State* vm, Port* p, Val x); // print method

/* Globals */
/* Global state objects */
extern State Vm;
extern Proc  Main;

#endif
