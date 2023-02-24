#include <string.h>

#include "runtime.h"

#include "util/collections.h"
#include "util/number.h"

// size padding ---------------------------------------------------------------
usize pad_stack_size(usize oldct, usize newct, usize oldcap, usize mincap) {
  (void)oldct;

  usize newcap = MAX(oldcap, mincap);

  if (newct <  newcap && (newct > (newcap >> 1) || newcap == mincap))
    return newcap;

  return ceil2(newct+1);
}

usize pad_table_size(usize oldct, usize newct, usize oldcap, usize mincap) {
  (void)oldct;
  usize newcap = MAX(oldcap, mincap);

  if (newct <  newcap * LoadF && (newct > (newcap >> 1) * LoadF || newcap == mincap))
    return newcap;

  return ceil2(newct * GrowF);
}

usize pad_alist_size(usize oldct, usize newct, usize oldcap, usize mincap) {
  usize newcap = MAX(oldcap, mincap);
  
  if (newcap > newct && (newct > (newcap >> 1) || newcap == mincap))
    return newcap;

  /* The over-allocation is mild, but is enough to give linear-time amortized 
   * behavior over a serial calls to push() in the presence of a poorly-performing
   * system realloc().
   *
   * Add padding to make the allocated size multiple of 4.
   * The growth pattern is:  0, 4, 8, 16, 24, 32, 40, 52, 64, 76, ...
   */
  
  newcap = (newct + (newct >> 3) + 6) & ~3ul;
  /* Do not overallocate if the new size is closer to overallocated size
   * than to the old size.
   */
  
  if (newct - oldct > newcap - newct)
    newcap = (newct + 3) & ~3ul;
  
  return newcap;
}

// alist apis -----------------------------------------------------------------
#undef ALIST_API

#define ALIST_API(A, X, a, padfn, minc)                                 \
  void init_##a(A* alist) {                                             \
    alist->len   = 0;                                                   \
    alist->cap   = padfn(0, 0, 0, minc);                                \
    alist->array = allocate(alist->cap, sizeof(X), 0);                  \
  }                                                                     \
  void free_##a(A* alist) {                                             \
    deallocate(alist->array, alist->cap, sizeof(X));                    \
    init_##a(alist);                                                    \
  }                                                                     \
  void resize_##a(A* alist, usize n) {                                  \
    usize c = padfn(alist->len, n, alist->cap, minc);                   \
    if (c != alist->cap) {                                              \
      alist->array = reallocate(alist->array, c, alist->cap, sizeof(X), 0); \
      alist->cap   = c;                                                 \
    }                                                                   \
  }                                                                     \
  usize a##_push(A* alist, X x) {                                       \
    resize_##a(alist, alist->len+1);                                    \
    alist->array[alist->len] = x;                                       \
    return alist->len++;                                                \
  }                                                                     \
  X a##_pop(A* alist) {                                                 \
    assert(alist->len > 0);                                             \
    X x = alist->array[--alist->len];                                   \
    resize_##a(alist, alist->len);                                      \
    return x;                                                           \
  }                                                                     \
  usize a##_write(A* alist, usize n, X* buf) {                          \
    usize off = alist->len;                                             \
    resize_##a(alist, alist->len+n);                                    \
    memcpy(alist->array+off, buf, n*sizeof(X));                         \
    alist->len += n;                                                    \
    return off;                                                         \
  }

ALIST_API(Vals, Val, vals, pad_alist_size, 1);
ALIST_API(Objs, Obj*, objs, pad_alist_size, 1);
ALIST_API(Bytes, ubyte, bytes, pad_stack_size, 8);
