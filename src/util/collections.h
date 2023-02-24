#ifndef util_collections_h
#define util_collections_h

#include "common.h"

// globals --------------------------------------------------------------------
#define MinSCap 8ul
#define MinTCap 8ul
#define MinACap 1ul
#define LoadF   0.625
#define GrowF   1.6

// size padding ---------------------------------------------------------------
usize pad_stack_size(usize oldct, usize newct, usize oldcap, usize mincap);
usize pad_alist_size(usize oldct, usize newct, usize oldcap, usize mincap);
usize pad_table_size(usize oldct, usize newct, usize oldcap, usize mincap);

// alist types ----------------------------------------------------------------
typedef uintptr_t  Val;
typedef struct Obj Obj;

#define ALIST(A, X)                             \
  typedef struct A {                            \
    X* array;                                   \
    usize len, cap;                             \
  } A

#define ALIST_API(A, X, a)                       \
  void  init_##a(A* alist);                      \
  void  free_##a(A* alist);                      \
  void  resize_##a(A* alist, usize n);           \
  usize a##_push(A* alist, X x);                 \
  X     a##_pop(A* alist);                       \
  usize a##_write(A* alist, usize n, X* buf)

ALIST(Vals, Val);
ALIST(Objs, Obj*);
ALIST(Bytes, ubyte);

ALIST_API(Vals, Val, vals);
ALIST_API(Objs, Obj*, objs);
ALIST_API(Bytes, ubyte, bytes);

#endif

