#ifndef rl_data_array_h
#define rl_data_array_h

/* Internal array types. */

// headers --------------------------------------------------------------------
#include "common.h"

// macros ---------------------------------------------------------------------
// array template
#define ALIST_API(A, X, a)                      \
  typedef struct {                              \
    X* vals;                                    \
    int count, max_count;                       \
  } A;                                          \
                                                \
  void init_##a(A* a);                          \
  void free_##a(A* a);                          \
  void grow_##a(A* a);                          \
  void shrink_##a(A* a);                        \
  void resize_##a(A* a, int n);                 \
  void a##_push(A* a, X x);                     \
  X    a##_pop(A* a);                           \
  void a##_write(A* a, X* xs, int n)

ALIST_API(Exprs, Expr, exprs);
ALIST_API(Objs, void*, objs);
ALIST_API(Bin16, ushort_t, bin16);

// C types --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void trace_exprs(Exprs* xs);
void trace_objs(Objs* os);

#endif
