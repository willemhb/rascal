#ifndef rl_runtime_h
#define rl_runtime_h

#include "values.h"

/* Just some preliminary typedefs and globals for runtime state. */
/* Types */
typedef struct RState RState;

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
extern struct List* ClArgs;
extern struct Map*  ClFlags;
extern struct Map*  ClOpts;


#endif
