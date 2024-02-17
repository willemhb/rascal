#ifndef rl_vm_memory_h
#define rl_vm_memory_h

#include "util/memory.h"

#include "val/object.h"

/* interface to rascal's memory management system. */

/* C types */
/* Structure for storing values in the C stack that need to be preserved but may not yet be visible from GC roots.
   
   This might occur if, for example, initializing a new object triggers a collection. */
struct GcFrame  {
  GcFrame* next;
  size_t   cnt;
  Value   *data;
};

/* Stores all the data necessary for proper memory management. */
struct RlHeap {
  size_t   heap_size, heap_cap;
  double   heap_load_factor;
  GcFrame* saved;
  Obj*     live_objects;
  Objects  gray_objects;
};

/* Globals */
extern RlHeap Heap;

/* External API */
void unsave_gc_frame(GcFrame* frame);

#define save(n, args...)                             \
  Value __gc_frame_vals[(n)] = { args };             \
  GcFrame __gc_frame cleanup(unsave_gc_frame) = {    \
    .next=Heap.saved,                                \
    .cnt =(n),                                       \
    .data=__gc_frame_vals                            \
  };                                                 \
  Heap.saved = &__gc_frame

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

void  add_to_heap(Obj* obj);
void  add_to_grays(Obj* obj);
void* alloc_obj(size_t n_bytes);
void  dealloc_obj(void* obj);
void* allocate(size_t n_bytes, bool use_heap);
void* duplicate(const void* pointer, size_t n_bytes, bool use_heap);
char* duplicates(char* chars, size_t n_chars, bool use_heap);
void* reallocate(void* pointer, size_t old_size, size_t new_size, bool use_heap);
void  deallocate(void* pointer, size_t n_bytes, bool use_heap);

#endif
