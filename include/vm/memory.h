#ifndef rl_vm_memory_h
#define rl_vm_memory_h

#include "vm/vm.h"

// heap management
void add_to_managed(RlState* rls, void* ptr);
void add_to_permanent(RlState* rls, void* ptr);
void gc_save(RlState* rls, void* ob);
void run_gc(RlState* rls);
void heap_report(RlState* rls);

// allocation
void* allocate(RlState* rls, size_t n);
char* duplicates(RlState* rls, char* cs);
void* duplicate(RlState* rls, size_t n, void* ptr);
void* reallocate(RlState* rls, size_t n, size_t o, void* spc);
void  release(RlState* rls, void* d, size_t n);

// misc GC helpers
void trace_expr_array(RlState* rls, Expr* exprs, int n);
void trace_obj_array(RlState* rls, Obj** objs, int n);

#endif
