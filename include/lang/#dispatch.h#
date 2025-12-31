#ifndef rl_lang_dispatch_h
#define rl_lang_dispatch_h

#include "val/fun.h"

/* main logic for handling multimethods - adding, resolving, and caching methods. */

// C types --------------------------------------------------------------------
// Globals --------------------------------------------------------------------
// Prototypes -----------------------------------------------------------------
// toplevel mutlimethod dispatch/add
void fun_add_method(RlState* rls, Fun* fun, Method* m);
void fun_add_method_s(RlState* rls, Fun* fun, Method* m);
Method* fun_get_method(RlState* rls, Fun* fun, int argc);
void add_builtin_method(RlState* rls, Fun* fun, OpCode op, bool va, int arity, ...);
void mtable_add(RlState* rls, MethodTable* mt, Method* m);
Method* mtable_dispatch(RlState* rls, MethodTable* mt, int argc); // arguments are on the stack

// other dispatch APIs
Tuple* get_signature(RlState* rls, int o, int n);
void init_mt_cache_table(RlState* rls, Table* table);
void init_mt_node_table(RlState* rls, Table* table);

// Helpers/Macros -------------------------------------------------------------

#endif
