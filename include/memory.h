#ifndef rascal_memory_h
#define rascal_memory_h

#include <assert.h>

#include "memutils.h"

#include "common.h"
#include "types.h"

// C types --------------------------------------------------------------------
/*
  memory map layout

   bits 0-4 - object repr
   bits 5-6 - gc flag
   bit  7   - traversal flag
 */

typedef enum {
  GCWHITE   = 0x00,
  GCBLACK   = 0x20,
  GCGRAY    = 0x40,
  GCMOVED   = 0x60,
  TRAVERSED = 0x80,

  // some values will never have their traversed flag set
  UNUSED    = REPR_SYMBOL | TRAVERSED,
  DATA      = REPR_STRING | TRAVERSED
} memfl_t;

typedef enum {
  RUNNING      =   1,
  RESIZING     =   2,
  SWAPPING     =   4,
  TRACING      =   8,
  RELOCATING   =  16,
  SWEEPING     =  32,
  FINALIZING   =  64,
} gcfl_t;

struct stack_t {
  uint_t   fp;
  ushort_t cap_min;
  ushort_t tag;

  uint_t   sp;
  uint_t   cap;

  value_t *data;
};

struct heap_t {
  uchar_t  align;    // allocation alignment
  uchar_t  word;     // word size
  uchar_t  minor;    // minor GC phase
  uchar_t  major;    // major GC phase
  uchar_t  stw;      // stop the world?
  uchar_t  grow : 4; // grow on next cycle
  uchar_t  grew : 4; // grew on last cycle
  ushort_t tag;

  float    grow_f;
  float    resize_f;

  size_t   cap;
  size_t   used;

  uchar_t *space;
  uchar_t *swap;
};
 
struct process_t {
  uint_t   idno;
  ushort_t flags;
  ushort_t tag;

  value_t  heap;
  value_t  stack;
  value_t  module;
  value_t  messages;
  value_t  tasks;
};

// utilities ------------------------------------------------------------------
// initial values --------------------------------------------------------------
#define N_STACK   4096
#define N_HEAP    8192
#define RESIZE_F  0.625
#define GROW_F    2.0

void    *allocate(size_t n, repr_t r);
void     collect_garbage(void);
value_t  relocate(value_t x);

uchar_t  get_mflags(value_t x);
uchar_t  set_mflags(value_t x, uchar_t fl);
uchar_t  clear_mflags(value_t x, uchar_t fl);

bool_t   is_white(value_t x);
bool_t   is_black(value_t x);
bool_t   is_gray(value_t x);
bool_t   is_moved(value_t x);
bool_t   is_traversed(value_t x);

index_t  push(value_t x);
void     dup(void);
value_t  pop(void);
index_t  pushn(size_t n);
value_t  popn(size_t n);

#define as_stack(x)           as_type(stack_t*, opval, x)
#define stack_sp(x)           getf(stack, x, sp)
#define stack_fp(x)           getf(stack, x, fp)
#define stack_cap_min(x)      getf(stack, x, cap_min)
#define stack_tag(x)          getf(stack, x, tag)
#define stack_cap(x)          getf(stack, x, cap)
#define stack_data(x)         getf(stack, x, data)

#define as_heap(x)         as_type(heap_t*, opval, x)
#define heap_tag(x)        getf(heap, x, tag)
#define heap_align(x)      getf(heap, x, align)
#define heap_word(x)       getf(heap, x, word)
#define heap_minor(x)      getf(heap, x, minor)
#define heap_major(x)      getf(heap, x, major)
#define heap_stw(x)        getf(heap, x, stw)
#define heap_grow_f(x)     getf(heap, x, grow_f)
#define heap_resize_f(x)   getf(heap, x, resize_f)
#define heap_cap(x)        getf(heap, x, cap)
#define heap_used(x)       getf(heap, x, used)
#define heap_space(x)      getf(heap, x, space)
#define heap_swap(x)       getf(heap, x, swap)

// implementation -------------------------------------------------------------

// native functions -----------------------------------------------------------

// initialization -------------------------------------------------------------
void memory_init(void);


#endif
