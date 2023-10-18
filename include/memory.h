#ifndef rascal_memory_h
#define rascal_memory_h

#include "object.h"
#include "vm.h"

// globals
#define N_HEAP  (((size_t)1)<<19)
#define HEAP_LF 0.625

// C types
struct GcFrame {
  GcFrame* next;
  size_t   cnt;
  Value*   vals;
};

// external API
void unsave_gc_frame(GcFrame* frame);

#define save(n, args...)                             \
  Value __gc_frame_vals[(n)] = { args };             \
  GcFrame __gc_frame cleanup(unsave_gc_frame) = {    \
    .next=RlVm.heap.frames,                          \
    .cnt =(n),                                       \
    .vals=__gc_frame_vals                            \
  }

#define SAFE_ALLOC(func, args...)                           \
  ({                                                        \
    void* __out = func(args);                               \
                                                            \
    if (__out == NULL) {                                    \
      fprintf(stderr,                                       \
              "Out of memory calling %s at %s:%s:%d.\n",    \
              #func,                                        \
              __FILE__,                                     \
              __func__,                                     \
              __LINE__);                                    \
      exit(1);                                              \
    }                                                       \
    __out;                                                  \
  })

#define SAFE_MALLOC(nBytes)           SAFE_ALLOC(malloc, nBytes)
#define SAFE_REALLOC(pointer, nBytes) SAFE_ALLOC(realloc, pointer, nBytes)

#define mark(p, ...)                                        \
  generic((p),                                              \
          Value:mark_val,                                   \
          Value*:mark_vals,                                 \
          void**:mark_objs,                                 \
          default:mark_obj)(p __VA_OPT__(,) __VA_ARGS__)

#define unmark(p, ...)                                      \
  generic((p),                                              \
          Value:unmark_val,                                 \
          Value*:unmark_vals,                               \
          void**:unmark_objs,                               \
          default:unmark_obj)(p __VA_OPT__(,) __VA_ARGS__)

void  mark_val(Value val);
void  mark_obj(void* ptr);
void  mark_vals(Value* vals, size_t n);
void  mark_objs(void** objs, size_t n);

void  unmark_val(Value val);
void  unmark_obj(void* obj);
void  unmark_vals(Value* vals, size_t n);
void  unmark_objs(void** objs, size_t n);

void  add_to_heap(void* p);

void  init_heap(Vm* vm);
void  free_heap(Vm* vm);

void* allocate(Vm* vm, size_t nBytes);
void* duplicate(Vm* vm, void* pointer, size_t n_bytes);
char* duplicates(Vm* vm, char* chars, size_t n_chars);
void* reallocate(Vm* vm, void* pointer, size_t old_size, size_t new_size);
void  deallocate(Vm* vm, void* pointer, size_t n_bytes);

#endif
