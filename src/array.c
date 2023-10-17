
#include "util/number.h"
#include "util/collection.h"

#include "memory.h"
#include "type.h"
#include "collection.h"
#include "array.h"

#undef ARRAY_API
#undef ARRAY_OBJ_API

#define ARRAY_API(A, X, _a, VA, ep, padFn)                              \
  void init##A(A* a) {                                                  \
    a->data = NULL;                                                     \
    a->cnt  = 0;                                                        \
    a->cap  = 0;                                                        \
  }                                                                     \
                                                                        \
  void free##A(A* a) {                                                  \
    deallocate(NULL, a->data, 0);                                       \
    init##A(a);                                                         \
  }                                                                     \
                                                                        \
  size_t resize##A(A* a, size_t newCnt) {                               \
    size_t oldCnt = a->cnt;                                             \
    size_t oldCap = a->cap;                                             \
    size_t newCap = padFn(oldCnt, newCnt, oldCap, ep);                  \
                                                                        \
    if (newCap == 0)                                                    \
      free##A(a);                                                       \
                                                                        \
    else if (newCap != oldCap) {                                        \
      size_t oldS  = oldCap * sizeof(X);                                \
      size_t newS  = newCap * sizeof(X);                                \
      a->data      = reallocate(NULL, a->data, oldS, newS);             \
      a->cap       = newCap;                                            \
    }                                                                   \
                                                                        \
    a->cnt = newCnt;                                                    \
    assert(a->cap >= a->cnt);                                           \
    return oldCnt;                                                      \
  }                                                                     \
                                                                        \
  size_t _a##Push(A* a, X x) {                                          \
    size_t off = resize##A(a, a->cnt+1);                                \
    a->data[off] = x;                                                   \
    return a->cnt-1;                                                    \
  }                                                                     \
                                                                        \
  size_t _a##Write(A* a, size_t n, X* d) {                              \
    size_t off = resize##A(a, a->cnt+n);                                \
    if (d != NULL)                                                      \
      memcpy(a->data+off, d, n*sizeof(X));                              \
    return off;                                                         \
  }                                                                     \
                                                                        \
  size_t _a##PushN(A* a, size_t n, ...) {                               \
    size_t off = resize##A(a, a->cnt+n);                                \
    va_list va; va_start(va, n);                                        \
                                                                        \
    for (size_t i=off; i<a->cnt; i++)                                   \
      a->data[i] = va_arg(va, VA);                                      \
                                                                        \
    va_end(va);                                                         \
    return off;                                                         \
  }                                                                     \
                                                                        \
  X _a##Pop(A* a) {                                                     \
    assert(a->cnt > 0);                                                 \
    X x = a->data[a->cnt-1];                                            \
    resize##A(a, a->cnt-1);                                             \
    return x;                                                           \
  }                                                                     \
                                                                        \
  void _a##PopN(A* a, size_t n) {                                       \
    assert(n <= a->cnt);                                                \
    if (a->cnt > 0 && n > 0)                                            \
      resize##A(a, a->cnt-n);                                           \
  }


#define ARRAY_OBJ_API(A, X, _a, VA, ep, padFn)                          \
  A* new##A(flags_t fl) {                                               \
    A* a = newObj(&A##Type, fl, 0);                                     \
    init##A(a);                                                         \
    return a;                                                           \
  }                                                                     \
                                                                        \
  void init##A(A* a) {                                                  \
    a->data = NULL;                                                     \
    a->cnt  = 0;                                                        \
    a->cap  = 0;                                                        \
  }                                                                     \
                                                                        \
  void free##A(void* p) {                                               \
    A* a = p;                                                           \
    deallocate(NULL, a->data, 0);                                       \
    init##A(a);                                                         \
  }                                                                     \
                                                                        \
  size_t resize##A(A* a, size_t newCnt) {                               \
    size_t oldCnt = a->cnt;                                             \
    size_t oldCap = a->cap;                                             \
    size_t newCap = padFn(oldCnt, newCnt, oldCap, ep);                  \
                                                                        \
    if (newCap == 0)                                                    \
      free##A(a);                                                       \
                                                                        \
    else if (newCap != oldCap) {                                        \
      size_t oldS  = oldCap * sizeof(X);                                \
      size_t newS  = newCap * sizeof(X);                                \
      a->data      = reallocate(NULL, a->data, oldS, newS);             \
      a->cap       = newCap;                                            \
    }                                                                   \
                                                                        \
    a->cnt = newCnt;                                                    \
    assert(a->cap >= a->cnt);                                           \
    return oldCnt;                                                      \
  }                                                                     \
                                                                        \
  size_t _a##Push(A* a, X x) {                                          \
    size_t off = resize##A(a, a->cnt+1);                                \
    a->data[off] = x;                                                   \
    return a->cnt-1;                                                    \
  }                                                                     \
                                                                        \
  size_t _a##Write(A* a, size_t n, X* d) {                              \
    size_t off = resize##A(a, a->cnt+n);                                \
    if (d != NULL)                                                      \
      memcpy(a->data+off, d, n*sizeof(X));                              \
    return off;                                                         \
  }                                                                     \
                                                                        \
  size_t _a##PushN(A* a, size_t n, ...) {                               \
    size_t off = resize##A(a, a->cnt+n);                                \
    va_list va; va_start(va, n);                                        \
                                                                        \
    for (size_t i=off; i<a->cnt; i++)                                   \
      a->data[i] = va_arg(va, VA);                                      \
                                                                        \
    va_end(va);                                                         \
    return off;                                                         \
  }                                                                     \
                                                                        \
  X _a##Pop(A* a) {                                                     \
    assert(a->cnt > 0);                                                 \
    X x = a->data[a->cnt-1];                                            \
    resize##A(a, a->cnt-1);                                             \
    return x;                                                           \
  }                                                                     \
                                                                        \
  void _a##PopN(A* a, size_t n) {                                       \
    assert(n <= a->cnt);                                                \
    if (a->cnt > 0 && n > 0)                                            \
      resize##A(a, a->cnt-n);                                           \
  }


ARRAY_API(Values, Value, values, Value, false, padAlistSize);
ARRAY_API(Objects, Obj*, objects, Obj*, false, padAlistSize);

ARRAY_OBJ_API(Binary8, byte_t, binary8, int, false, padAlistSize);
ARRAY_OBJ_API(Binary16, uint16_t, binary16, int, false, padAlistSize);
ARRAY_OBJ_API(Binary32, uint32_t, binary32, uint32_t, false, padAlistSize);
ARRAY_OBJ_API(Buffer8,  char,     buffer8,  int, true, padAlistSize);
ARRAY_OBJ_API(Buffer16, char16_t, buffer16, int, true, padAlistSize);
ARRAY_OBJ_API(Buffer32, char32_t, buffer32, int, true, padAlistSize);
ARRAY_OBJ_API(Alist, Value, alist, Value, false, padAlistSize);


// global type objects for above types
// Binary8 type
extern bool   equalBinary8s(Value x, Value y);
extern int    compareBinary8s(Value x, Value y);

Vtable Binary8Table = {
  .valSize=sizeof(Binary8*),
  .objSize=sizeof(Binary8),
  .tag    =OBJ_TAG,
  .free   =freeBinary8,
  .equal  =equalBinary8s,
  .order  =compareBinary8s
};

Type Binary8Type = {
  .obj={
    .type   =&TypeType,
    .annot  =&EmptyMap,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent=&TermType,
  .vTable=&Binary8Table,
  .idno  =BINARY8
};

// Binary16 type
extern bool   equalBinary16s(Value x, Value y);
extern int    compareBinary16s(Value x, Value y);

Vtable Binary16Table = {
  .valSize=sizeof(Binary16*),
  .objSize=sizeof(Binary16),
  .tag    =OBJ_TAG,
  .free   =freeBinary16,
  .equal  =equalBinary16s,
  .order  =compareBinary16s
};

Type Binary16Type = {
  .obj={
    .type   =&TypeType,
    .annot  =&EmptyMap,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent=&TermType,
  .vTable=&Binary16Table,
  .idno  =BINARY16
};

// Binary32 type
extern bool   equalBinary32s(Value x, Value y);
extern int    compareBinary32s(Value x, Value y);

Vtable Binary32Table = {
  .valSize=sizeof(Binary32*),
  .objSize=sizeof(Binary32),
  .tag    =OBJ_TAG,
  .free   =freeBinary32,
  .equal  =equalBinary32s,
  .order  =compareBinary32s
};

Type Binary32Type = {
  .obj={
    .type   =&TypeType,
    .annot  =&EmptyMap,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent=&TermType,
  .vTable=&Binary32Table,
  .idno  =BINARY32
};

// Buffer8 type
extern bool   equalBuffer8s(Value x, Value y);
extern int    compareBuffer8s(Value x, Value y);

Vtable Buffer8Table = {
  .valSize=sizeof(Buffer8*),
  .objSize=sizeof(Buffer8),
  .tag    =OBJ_TAG,
  .free   =freeBuffer8,
  .equal  =equalBuffer8s,
  .order  =compareBuffer8s
};

Type Buffer8Type = {
  .obj={
    .type   =&TypeType,
    .annot  =&EmptyMap,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent=&TermType,
  .vTable=&Buffer8Table,
  .idno  =BUFFER8
};

// Buffer16 type
extern bool   equalBuffer16s(Value x, Value y);
extern int    compareBuffer16s(Value x, Value y);

Vtable Buffer16Table = {
  .valSize=sizeof(Buffer16*),
  .objSize=sizeof(Buffer16),
  .tag    =OBJ_TAG,
  .free   =freeBuffer16,
  .equal  =equalBuffer16s,
  .order  =compareBuffer16s
};

Type Buffer16Type = {
  .obj={
    .type   =&TypeType,
    .annot  =&EmptyMap,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent=&TermType,
  .vTable=&Buffer16Table,
  .idno  =BUFFER16
};

// Buffer32 type
extern bool   equalBuffer32s(Value x, Value y);
extern int    compareBuffer32s(Value x, Value y);

Vtable Buffer32Table = {
  .valSize=sizeof(Buffer32*),
  .objSize=sizeof(Buffer32),
  .tag    =OBJ_TAG,
  .free   =freeBuffer32,
  .equal  =equalBuffer32s,
  .order  =compareBuffer32s
};

Type Buffer32Type = {
  .obj={
    .type   =&TypeType,
    .annot  =&EmptyMap,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent=&TermType,
  .vTable=&Buffer32Table,
  .idno  =BUFFER32
};

// Alist type
extern void   traceAlist(void* p);

Vtable AlistTable = {
  .valSize=sizeof(Alist*),
  .objSize=sizeof(Alist),
  .tag    =OBJ_TAG,
  .free   =freeAlist,
  .trace  =traceAlist
};

Type AlistType = {
  .obj={
    .type   =&TypeType,
    .annot  =&EmptyMap,
    .noSweep=true,
    .noFree =true,
    .gray   =true,
  },
  .parent=&TermType,
  .vTable=&AlistTable,
  .idno  =ALIST
};
