
/* Implementations for internal dynamic array types. */

// headers --------------------------------------------------------------------
#include "data/array.h"

#include "sys/error.h"
#include "sys/memory.h"

// magic numbers
#define MIN_CAP 8
#define LOADF   0.625

// Binary API
// object for storing binary data
// alist implementation macro
#define ALIST_IMPL(A, X, a)                                         \
  void init_##a(A* a) {                                             \
    a->vals      = NULL;                                            \
    a->count     = 0;                                               \
    a->max_count = 0;                                               \
  }                                                                 \
                                                                    \
  void free_##a(A* a) {                                             \
    release(a->vals, 0);                                            \
    init_##a(a);                                                    \
  }                                                                 \
                                                                    \
  void grow_##a(A* a) {                                             \
    if ( a->max_count == MAX_ARITY )                                \
      runtime_error("maximum "#a" size exceeded");                  \
    int new_maxc  = a->max_count ? a->max_count << 1 : MIN_CAP;   \
    X*  new_spc  = reallocate(false,                              \
                              new_maxc * sizeof(X),               \
                              a->max_count * sizeof(X),           \
                              a->vals);                           \
                                                                  \
    a->vals = new_spc;                                            \
    a->max_count = new_maxc;                                      \
  }                                                               \
                                                                  \
  void shrink_##a(A* a) {                                         \
    assert(a->max_count > MIN_CAP);                               \
                                                                  \
    size_t new_maxc = a->max_count >> 1;                          \
    X*  new_spc     = reallocate(false,                           \
                                 new_maxc*sizeof(X),              \
                                 a->max_count*sizeof(X),          \
                                 a->vals);                        \
                                                                  \
    a->vals      = new_spc;                                       \
    a->max_count = new_maxc;                                      \
  }                                                               \
                                                                  \
  void resize_##a(A* a, int n) {                                  \
    if ( n > MAX_ARITY )                                          \
      runtime_error("maximum"#a"size exceeded");                  \
                                                                        \
    int new_maxc = cpow2(n);                                            \
                                                                        \
    if ( new_maxc < MIN_CAP )                                           \
      new_maxc = MIN_CAP;                                               \
                                                                        \
    X* new_spc = reallocate(false,                                      \
                            new_maxc*sizeof(X),                         \
                            a->max_count*sizeof(X),                     \
                            a->vals);                                   \
    a->vals         = new_spc;                                          \
    a->max_count    = new_maxc;                                         \
  }                                                                     \
                                                                        \
  void a##_push(A* a, X x) {                                            \
    if ( a->count == a->max_count )                                     \
      grow_##a(a);                                                      \
                                                                        \
    a->vals[a->count++] = x;                                            \
  }                                                                     \
                                                                        \
  X a##_pop(A* a) {                                                     \
    X out = a->vals[--a->count];                                        \
                                                                        \
    if ( a->count == 0 )                                                \
      free_##a(a);                                                      \
                                                                        \
    else if ( a->max_count > MIN_CAP && a->count < (a->max_count >> 1) ) \
      shrink_##a(a);                                                    \
                                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
                                                                        \
  void a##_write(A* a, X* xs, int n) {                                  \
    if ( a->count + n > a->max_count )                                  \
      resize_##a(a, a->count+n);                                        \
                                                                        \
    if ( xs != NULL )                                                   \
      memcpy(a->vals+a->count, xs, n*sizeof(X));                        \
                                                                        \
    a->count += n;                                                      \
  }

ALIST_IMPL(Exprs, Expr, exprs);
ALIST_IMPL(Objs, void*, objs);
ALIST_IMPL(Bin16, ushort_t, bin16);

