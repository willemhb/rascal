#ifndef rl_vm_memory_h
#define rl_vm_memory_h

#include "vm/context.h"

/* interface to rascal's memory management system. */
void  add_to_heap(RlCtx* ctx, Obj* obj);
void  add_to_grays(RlCtx* ctx, Obj* obj);
void* allocate(RlCtx* ctx, size_t n_bytes);
void* duplicate(RlCtx* ctx, void* pointer, size_t n_bytes);
char* duplicates(RlCtx* ctx, char* chars, size_t n_chars);
void* reallocate(RlCtx* ctx, void* pointer, size_t old_size, size_t new_size);
void  deallocate(RlCtx* ctx, void* pointer, size_t n_bytes);

#endif
