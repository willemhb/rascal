#include "util/number.h"
#include "util/collection.h"

#include "runtime.h"

#include "type.h"
#include "function.h"
#include "collection.h"

#include "array.h"

#undef ARRAY_API

#define ARRAY_API(A, X, _a, VA, ep, pad_fn)                             \
  A* new_##_a(flags_t fl) {                                             \
    A* a = new_obj(&A##Type, fl, 0);                                    \
    init_##_a(a);                                                       \
    return a;                                                           \
  }                                                                     \
                                                                        \
  void init_##_a(A* a) {                                                \
    a->data = NULL;                                                     \
    a->cnt  = 0;                                                        \
    a->cap  = 0;                                                        \
  }                                                                     \
                                                                        \
  void free_##_a(void* p) {                                             \
    A* a = p;                                                           \
    deallocate(NULL, a->data, 0);                                       \
    init_##_a(a);                                                       \
  }                                                                     \
                                                                        \
  size_t resize##A(A* a, size_t new_cnt) {                              \
    size_t old_cnt = a->cnt;                                            \
    size_t old_cap = a->cap;                                            \
    size_t new_cap = pad_fn(old_cnt, new_cnt, old_cap, ep);             \
                                                                        \
    if (new_cap == 0)                                                   \
      free_##_a(a);                                                     \
                                                                        \
    else if (new_cap != old_cap) {                                      \
      size_t old_s = old_cap * sizeof(X);                               \
      size_t new_s = new_cap * sizeof(X);                               \
      a->data      = reallocate(NULL, a->data, old_s, new_s);           \
      a->cap       = new_cap;                                           \
    }                                                                   \
                                                                        \
    a->cnt = new_cnt;                                                   \
    assert(a->cap >= a->cnt);                                           \
    return old_cnt;                                                     \
  }                                                                     \
                                                                        \
  X*     _a##_peek(A* a, int i) {                                       \
    if (i < 0)                                                          \
      i += (int)a->cnt;                                                 \
                                                                        \
    assert(i >= 0 && i < (int)a->cnt);                                  \
    return &a->data[i];                                                 \
  }                                                                     \
                                                                        \
  size_t _a##_push(A* a, X x) {                                         \
    size_t off = resize_##_a(a, a->cnt+1);                              \
    a->data[off] = x;                                                   \
    return a->cnt-1;                                                    \
  }                                                                     \
                                                                        \
  size_t _a##_write(A* a, size_t n, X* d) {                             \
    size_t off = resize_##_a(a, a->cnt+n);                              \
    if (d != NULL)                                                      \
      memcpy(a->data+off, d, n*sizeof(X));                              \
    return off;                                                         \
  }                                                                     \
                                                                        \
  size_t _a##_pushn(A* a, size_t n, ...) {                              \
    size_t off = resize_##_a(a, a->cnt+n);                              \
    va_list va; va_start(va, n);                                        \
                                                                        \
    for (size_t i=off; i<a->cnt; i++)                                   \
      a->data[i] = va_arg(va, VA);                                      \
                                                                        \
    va_end(va);                                                         \
    return off;                                                         \
  }                                                                     \
                                                                        \
  X _a##_pop(A* a) {                                                    \
    assert(a->cnt > 0);                                                 \
    X x = a->data[a->cnt-1];                                            \
    resize_##_a(a, a->cnt-1);                                           \
    return x;                                                           \
  }                                                                     \
                                                                        \
  void _a##_popn(A* a, size_t n) {                                      \
    assert(n <= a->cnt);                                                \
    if (a->cnt > 0 && n > 0)                                            \
      resize_##_a(a, a->cnt-n);                                         \
  }

ARRAY_API(Binary8, byte_t, binary8, int, false, pad_alist_size);
ARRAY_API(Binary16, uint16_t, binary16, int, false, pad_alist_size);
ARRAY_API(Binary32, uint32_t, binary32, uint32_t, false, pad_alist_size);
ARRAY_API(Buffer8,  char,     buffer8,  int, true, pad_alist_size);
ARRAY_API(Buffer16, char16_t, buffer16, int, true, pad_alist_size);
ARRAY_API(Buffer32, char32_t, buffer32, int, true, pad_alist_size);
ARRAY_API(Alist, Value, alist, Value, false, pad_alist_size);

// global type objects for above types
// Binary8 type
extern bool   equal_binary8s(Value x, Value y);
extern int    compare_binary8s(Value x, Value y);

Vtable Binary8Table = {
  .val_size=sizeof(Binary8*),
  .obj_size=sizeof(Binary8),
  .tag    =OBJ_TAG,
  .free   =free_binary8,
  .equal  =equal_binary8s,
  .order  =compare_binary8s
};

Function Binary8Ctor = {
  .obj={
    .type    =&FunctionType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .gray    =true,
  }
};

Type Binary8Type = {
  .obj={
    .type   =&TypeType,
    .annot  =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
  },
  .parent =&TermType,
  .ctor   =&Binary8Ctor,
  .v_table=&Binary8Table,
  .idno   =BINARY8
};

// Binary16 type
extern bool equal_binary16s(Value x, Value y);
extern int  compare_binary16s(Value x, Value y);

Vtable Binary16Table = {
  .val_size=sizeof(Binary16*),
  .obj_size=sizeof(Binary16),
  .tag     =OBJ_TAG,
  .free    =free_binary16,
  .equal   =equal_binary16s,
  .order   =compare_binary16s
};

Function Binary16Ctor = {
  .obj={
    .type    =&FunctionType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .gray    =true,
  }
};

Type Binary16Type = {
  .obj={
    .type   =&TypeType,
    .annot  =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray   =true,
  },
  .parent =&TermType,
  .ctor   =&Binary16Ctor,
  .v_table=&Binary16Table,
  .idno   =BINARY16
};

// Binary32 type
extern bool   equal_binary32s(Value x, Value y);
extern int    compare_binary32s(Value x, Value y);

Vtable Binary32Table = {
  .val_size=sizeof(Binary32*),
  .obj_size=sizeof(Binary32),
  .tag     =OBJ_TAG,
  .free    =free_binary32,
  .equal   =equal_binary32s,
  .order   =compare_binary32s
};

Function Binary32Ctor = {
  .obj={
    .type    =&FunctionType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .gray    =true,
  }
};

Type Binary32Type = {
  .obj={
    .type    =&TypeType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
  },
  .parent =&TermType,
  .ctor   =&Binary32Ctor,
  .v_table=&Binary32Table,
  .idno   =BINARY32
};

// Buffer8 type
extern bool equal_buffer8s(Value x, Value y);
extern int  compare_buffer8s(Value x, Value y);

Vtable Buffer8Table = {
  .val_size=sizeof(Buffer8*),
  .obj_size=sizeof(Buffer8),
  .tag     =OBJ_TAG,
  .free    =free_buffer8,
  .equal   =equal_buffer8s,
  .order   =compare_buffer8s
};

Function Buffer8Ctor = {
  .obj={
    .type    =&FunctionType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .gray    =true,
  }
};

Type Buffer8Type = {
  .obj={
    .type    =&TypeType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
  },
  .parent =&TermType,
  .ctor   =&Buffer8Ctor,
  .v_table=&Buffer8Table,
  .idno   =BUFFER8
};

// Buffer16 type
extern bool equal_buffer16s(Value x, Value y);
extern int  compare_buffer16s(Value x, Value y);

Vtable Buffer16Table = {
  .val_size=sizeof(Buffer16*),
  .obj_size=sizeof(Buffer16),
  .tag     =OBJ_TAG,
  .free    =free_buffer16,
  .equal   =equal_buffer16s,
  .order   =compare_buffer16s
};

Function Buffer16Ctor = {
  .obj={
    .type    =&FunctionType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .gray    =true,
  }
};

Type Buffer16Type = {
  .obj={
    .type    =&TypeType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
  },
  .parent =&TermType,
  .ctor   =&Buffer16Ctor,
  .v_table=&Buffer16Table,
  .idno   =BUFFER16
};

// Buffer32 type
extern bool equal_buffer32s(Value x, Value y);
extern int  compare_buffer32s(Value x, Value y);

Vtable Buffer32Table = {
  .val_size=sizeof(Buffer32*),
  .obj_size=sizeof(Buffer32),
  .tag     =OBJ_TAG,
  .free    =free_buffer32,
  .equal   =equal_buffer32s,
  .order   =compare_buffer32s
};

Function Buffer32Ctor = {
  .obj={
    .type    =&FunctionType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .gray    =true,
  }
};

Type Buffer32Type = {
  .obj={
    .type    =&TypeType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
  },
  .parent =&TermType,
  .ctor   =&Buffer32Ctor,
  .v_table=&Buffer32Table,
  .idno   =BUFFER32
};

// Alist type
extern void trace_alist(void* p);

Vtable AlistTable = {
  .val_size=sizeof(Alist*),
  .obj_size=sizeof(Alist),
  .tag     =OBJ_TAG,
  .free    =free_alist,
  .trace   =trace_alist
};

Function AlistCtor = {
  .obj={
    .type    =&FunctionType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .gray    =true,
  }
};

Type AlistType = {
  .obj={
    .type    =&TypeType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
  },
  .parent =&TermType,
  .ctor   =&AlistCtor,
  .v_table=&AlistTable,
  .idno   =ALIST
};

// Objects type
extern void trace_objects(void* p);

Vtable ObjectsTable = {
  .val_size=sizeof(Objects*),
  .obj_size=sizeof(Objects),
  .tag     =OBJ_TAG,
  .free    =free_objects,
  .trace   =trace_objects
};

Function ObjectsCtor = {
  .obj={
    .type    =&FunctionType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .gray    =true,
  }
};

Type ObjectsType = {
  .obj={
    .type    =&TypeType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
  },
  .parent =&TermType,
  .ctor   =&ObjectsCtor,
  .v_table=&ObjectsTable,
  .idno   =OBJECTS
};
