#ifndef rl_sys_memory_h
#define rl_sys_memory_h

/**
 *
 * Data structures and definitions for Rascal memory management.
 * 
 **/

// headers --------------------------------------------------------------------
#include "sys/base.h"

// macros ---------------------------------------------------------------------
// various helpers for preserving values in the C stack during gc
#define preserve(n, vals...)                                            \
  Expr __gc_frame_vals__[(n)] = { vals };                               \
  GcFrame __gc_frame__ __attribute__((__cleanup__(next_gc_frame))) = {  \
    .next  = GcFrames,                                                  \
    .count =  (n),                                                      \
    .exprs = __gc_frame_vals__                                          \
  };                                                                    \
    GcFrames = &__gc_frame__

#define add_to_preserved(n, x)                  \
  __gc_frame_vals__[(n)] = (x)

#define preserved() __gc_frame_vals__

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
// high level heap API --------------------------------------------------------
void* allocate(RascalState* s, bool h, size_t n);
void* duplicate(RascalState* s, bool h, size_t n, void* ptr);
void* reallocate(RascalState* s, bool h, size_t n, size_t o, void* spc);
void  release(RascalState* s, void* d, size_t n);
void  run_gc(RascalState* s);

// low level heap helpers -----------------------------------------------------
void add_to_heap(RascalState* s, void* ptr);
void gc_save(RascalState* s, void* ob);
void trace_exp_array(RascalState* s, size_t n, Expr* xs);
void trace_obj_array(RascalState* s, size_t n, void* os);
void heap_report(RascalState* s);

// initialization -------------------------------------------------------------
void toplevel_init_sys_memory(void);

#endif
