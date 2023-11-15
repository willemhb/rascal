#ifndef rl_vm_memory_h
#define rl_vm_memory_h

#include "vm/context.h"

/* interface to rascal's memory management system. */

void unsave_gc_frame(GcFrame* frame);
void unsave_hamt_buf(HamtBuf* htbuf);

#define save(n, args...)                             \
  Value __gc_frame_vals[(n)] = { args };             \
  GcFrame __gc_frame cleanup(unsave_gc_frame) = {    \
    .next=Ctx.gcframes,                              \
    .cnt =(n),                                       \
    .data=__gc_frame_vals                            \
  };                                                 \
  Ctx.gcframes = &__gc_frame

#define add_saved(n, val) __gc_frame_vals[(n)] = (val)

void  add_to_heap(RlCtx* ctx, Obj* obj);
void  add_to_grays(RlCtx* ctx, Obj* obj);
bool  alloc_will_overflow(RlCtx* ctx, size_t n_bytes);
bool  new_obj_will_overflow(Type* type);
void* allocate(RlCtx* ctx, size_t n_bytes);
void* duplicate(RlCtx* ctx, void* pointer, size_t n_bytes);
char* duplicates(RlCtx* ctx, char* chars, size_t n_chars);
void* reallocate(RlCtx* ctx, void* pointer, size_t old_size, size_t new_size);
void  deallocate(RlCtx* ctx, void* pointer, size_t n_bytes);

#endif
