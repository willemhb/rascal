#ifndef rascal_memory_h
#define rascal_memory_h

#include <assert.h>

#include "common.h"

// C types --------------------------------------------------------------------
struct stack_t
{
  value_t *data;

  uint sp, cap;
};

struct heap_t
{
  size_t used, available, alignment;

  bool grow, grew, collecting;

  uchar *space, *swap;
};

struct gc_frame_t
{
  size_t      length;
  gc_frame_t *next;
  value_t    **saved;
};

// macros ---------------------------------------------------------------------
#define RSP_GC_FRAME gc_frame_t __gc_frame__ __attribute__((cleanup(gc_frame_cleanup)))

#define preserve(n, vars...)					\
  value_t *__gc_frame_array__[(n)] = { vars };			\
  RSP_GC_FRAME = { (n), Saved, &__gc_frame_array__[0] };	\
  Saved = &__gc_frame__

#endif
