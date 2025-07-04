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
void* allocate(bool h, size_t n);
char* duplicates(char* cs);
void* duplicate(bool h, size_t n, void* ptr);
void* reallocate(bool h, size_t n, size_t o, void* spc);
void  release(void* d, size_t n);
void  run_gc(void);

// low level heap helpers -----------------------------------------------------
void  add_to_heap(void* ptr);
void  gc_save(void* ob);
void  next_gc_frame(GcFrame* gcf);
void  heap_report(void);

// initialization -------------------------------------------------------------
void toplevel_init_sys_memory(void);

#endif
