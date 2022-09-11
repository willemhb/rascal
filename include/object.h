#ifndef rascal_object_h
#define rascal_object_h

#include "types.h"

// C types --------------------------------------------------------------------
struct object_t
{
  uint   arity;
  ushort size;
  ushort flags   :  8;
  ushort tag     :  3;
  uchar  gc      :  2;
  uchar  guarded :  1;
  uchar  moved   :  1;
  uchar  seen    :  1;

  uchar  space[0];
};

// utility macros & statics ---------------------------------------------------
#define as_object(x)  asa(object_t*, x, pval)

#define ob_arity(x)   getf(object_t*, x, arity)
#define ob_size(x)    getf(object_t*, x, size)
#define ob_flags(x)   getf(object_t*, x, flags)
#define ob_tag(x)     getf(object_t*, x, tag)
#define ob_gc(x)      getf(object_t*, x, gc)
#define ob_guarded(x) getf(object_t*, x, guarded)
#define ob_moved(x)   getf(object_t*, x, moved)
#define ob_seen(x)    getf(object_t*, x, seen)

#define ob_space(x)   (&(getf(object_t*, x, space)[0]))

static inline bool is_immediate(value_t x)
{
  return (x&QNAN) == QNAN
    ||   !(x&PMASK)
    ||   !!(x&15);
}

static inline bool is_object(value_t x)
{
  return !is_immediate(x);
}

static inline void init_object(void *ob, size_t size, uint tag, uint flags)
{
  object_t *o   = ob;

  ob_arity(o)   = 0;
  ob_size(o)    = size;
  ob_tag(o)     = tag;
  ob_flags(o)   = flags;
  ob_gc(o)      = 0;
  ob_guarded(o) = false;
  ob_moved(o)   = false;
  ob_seen(o)    = false;
}

#endif
