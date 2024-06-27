#ifndef rl_runtime_h
#define rl_runtime_h

#include "values.h"

/* Just some preliminary typedefs and globals for runtime state. */
/* Frame types */
typedef struct HFrame HFrame;
typedef struct RFrame RFrame;
typedef struct CFrame CFrame;
typedef struct IFrame IFrame;

/* State types */
typedef struct HState HState;
typedef struct RState RState;
typedef struct CState CState;
typedef struct IState IState;
typedef struct EState EState;

// internal function pointer types
typedef void     (*rl_trace_fn_t)(void* x);
typedef void     (*rl_free_fn_t)(void* x);
typedef void     (*rl_clone_fn_t)(void* x);
typedef void     (*rl_read_fn_t)(RState* s, int d);
typedef hash_t   (*rl_hash_fn_t)(Val x);
typedef bool     (*rl_egal_fn_t)(Val x, Val y);
typedef int      (*rl_order_fn_t)(Val x, Val y);
typedef size_t   (*rl_sizeof_fn_t)(void* x);
typedef rl_err_t (*rl_native_fn_t)(size_t n, Val* a, Val* b);

/* Globals */
/* Parsed command line arguments and options. */
extern List* ClArgs;
extern Set*  ClFlags;
extern Map*  ClOpts;

/* Global state objects */
extern HState Heap;
extern RState Reader;
extern CState Compiler;
extern IState Vm;
extern EState Environ;

/* Standard streams */
extern Port StdIn;
extern Port StdOut;
extern Port StdErr;

/* Empty singletons */

#endif
