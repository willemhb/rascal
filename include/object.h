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
    ||   !!(x&7);
}

static inline bool is_object(value_t x)
{
  return !is_immediate(x);
}

#endif
