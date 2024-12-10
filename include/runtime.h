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
typedef struct VTable   VTable;   // type information object

// signature for C functions that implement primitive functions
typedef void   (*CPrimFn)(Proc* p, int n);               // handler for PrimFn

// Object interface signatures
typedef void   (*TraceFn)(State* vm, void* x);           // trace lifetime method
typedef void   (*FreeFn)(State* vm, void* x);            // free lifetime method
typedef void   (*CloneFn)(State* vm, void* x, bool d);   // clone lifetime method (only responsible for duplicating owned data)
typedef void   (*SealFn)(State* vm, void* x, bool d);    // seal lifetime method
typedef Box*   (*BoxFn)(State* vm, Val d);               // called to box an immediate value
typedef size64 (*UnboxFn)(Val d, void* b, size64 n);     // called to unbox a value with an immediate representation

// Comparison interface signatures
typedef hash64 (*HashFn)(Val x);                      // hash method
typedef bool   (*EgalFn)(Val x, Val y);               // egal method
typedef int    (*OrderFn)(Val x, Val y);              // order method

/* Globals */
// Global state objects
extern State Vm;
extern Proc  Main;

#endif
