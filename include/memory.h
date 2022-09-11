#ifndef rascal_memory_h
#define rascal_memory_h

#include "object.h"
#include "function.h"

// C types --------------------------------------------------------------------
typedef struct gc_frame_t
{
  size_t size;
  struct gc_frame_t *next;
  struct gc_frame_t *catch;
  value_t **saved;
} gc_frame_t;

// globals --------------------------------------------------------------------
extern uchar *Heap, *Swap;

extern size_t HeapSize, HeapUsed, SwapUsed;

extern bool Grow, Grew, Collecting;

extern gc_frame_t  *Saved, *Catch;

// macros ---------------------------------------------------------------------
#define preserve(n, save...)						\
  value_t *__addr__[(n)] = { save };					\
  gc_frame_t __gc__ __attribute__((cleanup(cleanup_gc_frames))) =	\
    { (n), Saved, __addr__ };						\
  Saved = &__gc__

// forward declarations -------------------------------------------------------
void     cleanup_gc_frames(gc_frame_t *gcf);
void    *allocate(size_t n, bool gl);

bool     global_val(value_t x);
bool     global_ob(void *o);

value_t  move_val(value_t x);
void    *move_ob(void *o);

#define move(x)      _Generic((x), value_t: move_val, default: move_ob)(x)
#define is_global(x) _Generic((x), value_t: global_val, default: global_ob)(x)

#endif
