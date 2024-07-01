#ifndef rl_runtime_h
#define rl_runtime_h

#include "values.h"

/* Just some preliminary typedefs and globals for runtime state. */
/* Frame types */
typedef struct HFrame HFrame;

/* State types */
typedef struct RlState RlState;
typedef struct RlProc  RlProc;

// internal function pointer types
typedef void   (*TraceFn)(void* x);
typedef void   (*FreeFn)(void* x);
typedef void   (*CloneFn)(void* x);
typedef hash_t (*HashFn)(Val x);
typedef bool   (*EgalFn)(Val x, Val y);
typedef int    (*OrderFn)(Val x, Val y);
typedef size_t (*SizeofFn)(void* x);
typedef void   (*NativeFn)(RlState* s, bool c);

/* Globals */
/* Parsed command line arguments and options. */
extern List* ClArgs;
extern Set*  ClFlags;
extern Map*  ClOpts;

/* Global state objects */
extern RlState Vm;         // main Vm state object
extern RlProc  Main;       // main process (for the time being this is the only process)

/* Standard streams */
extern Port StdIn;
extern Port StdOut;
extern Port StdErr;

/* Empty singletons */

#endif
