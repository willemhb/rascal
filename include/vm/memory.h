#ifndef rl_vm_memory_h
#define rl_vm_memory_h

#include "util/memory.h"

#include "vm/context.h"

/* interface to rascal's memory management system. */

/* External API */
void unsave_gc_frame(GcFrame* frame);

#define save(n, args...)                             \
  Value __gc_frame_vals[(n)] = { args };             \
  GcFrame __gc_frame cleanup(unsave_gc_frame) = {    \
    .next=Ctx.h.frames,                              \
    .cnt =(n),                                       \
    .data=__gc_frame_vals                            \
  };                                                 \
  Ctx.h.frames = &__gc_frame

#define add_saved(n, val) __gc_frame_vals[(n)] = (val)

void trace_val(Value val);
void trace_obj(void* obj);

void mark_val(Value val);
void mark_obj(void* ptr);
void mark_vals(Value* vals, size_t n);
void mark_objs(Obj** objs, size_t n);

void unmark_val(Value val);
void unmark_obj(void* obj);
void unmark_vals(Value* vals, size_t n);
void unmark_objs(Obj** objs, size_t n);

#define trace(x) generic2(trace, x, x)

#define mark(p, ...)                                        \
  generic((p),                                              \
          Value:mark_val,                                   \
          Value*:mark_vals,                                 \
          Obj**:mark_objs,                                  \
          default:mark_obj)(p __VA_OPT__(,) __VA_ARGS__)

#define unmark(p, ...)                                      \
  generic((p),                                              \
          Value:unmark_val,                                 \
          Value*:unmark_vals,                               \
          Obj**:unmark_objs,                               \
          default:unmark_obj)(p __VA_OPT__(,) __VA_ARGS__)

void  add_to_heap(RlCtx* ctx, Obj* obj);
void  add_to_grays(RlCtx* ctx, Obj* obj);
bool  alloc_will_overflow(RlCtx* ctx, size_t n_bytes);
bool  new_obj_will_overflow(Type* type);
void* allocate(RlCtx* ctx, size_t n_bytes);
void* duplicate(RlCtx* ctx, void* pointer, size_t n_bytes);
char* duplicates(RlCtx* ctx, char* chars, size_t n_chars);
void* reallocate(RlCtx* ctx, void* pointer, size_t old_size, size_t new_size);
void  deallocate(RlCtx* ctx, void* pointer, size_t n_bytes);

/* Initialization */
void  vm_init_memory(void);

#endif
