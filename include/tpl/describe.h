#ifndef rascal_template_describe_h
#define rascal_template_describe_h

#include "util/number.h"

#include "object.h"
#include "memory.h"

#undef ARRAY_API
#undef TABLE_API
#undef ARRAY_OBJ_API
#undef TABLE_OBJ_API

#define ARRAY_API(A, X, _a, VA, ep)                                     \
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
    size_t newPad = newCnt + ep;                                        \
    if (newCnt == 0) {                                                  \
      if (a->cnt != 0)                                                  \
        free##A(a);                                                     \
    } else if (newPad > a->cap || newPad < (a->cap >> 1)) {             \
      size_t oldCap   = a->cap;                                         \
      size_t newCap   = max(8u, ceilPow2(newPad+1));                    \
                                                                        \
      if (oldCap != newCap) {                                           \
        size_t oldSize  = oldCap * sizeof(X);                           \
        size_t newSize  = newCap * sizeof(X);                           \
        if (a->data == NULL)                                            \
          a->data = allocate(NULL, newSize);                            \
        else                                                            \
          a->data = reallocate(NULL, a->data, oldSize, newSize);        \
        a->cap = newCap;                                                \
      }                                                                 \
    }                                                                   \
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

#define ARRAY_OBJ_API(A, X, _a, VA, ep)                                 \
  A* new##A(int fl) {                                                   \
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
    size_t newPad = newCnt + ep;                                        \
    if (newCnt == 0) {                                                  \
      if (a->cnt != 0)                                                  \
        free##A(a);                                                     \
    } else if (newPad > a->cap || newPad < (a->cap >> 1)) {             \
      size_t oldCap   = a->cap;                                         \
      size_t newCap   = max(8u, ceilPow2(newPad+1));                    \
                                                                        \
      if (oldCap != newCap) {                                           \
        size_t oldSize  = oldCap * sizeof(X);                           \
        size_t newSize  = newCap * sizeof(X);                           \
        if (a->data == NULL)                                            \
          a->data = allocate(NULL, newSize);                            \
        else                                                            \
          a->data = reallocate(NULL, a->data, oldSize, newSize);        \
        a->cap = newCap;                                                \
      }                                                                 \
    }                                                                   \
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

#define TABLE_MAX_LOAD 0.75

#define TABLE_API(T, K, V, _t, cmp, hashK, noK, noV)                    \
  void init##T(T* t) {                                                  \
    t->kvs   = NULL;                                                    \
    t->cnt   = 0;                                                       \
    t->cap   = 0;                                                       \
  }                                                                     \
                                                                        \
  void free##T(T* t) {                                                  \
    deallocate(NULL, t->kvs, 0);                                        \
    init##T(t);                                                         \
  }                                                                     \
                                                                        \
  static T##Kv* find##T##Kv(T##Kv* kvs, size_t cap, K k) {              \
    uint64_t hash    = hashK(k);                                        \
    uint64_t mask    = cap - 1;                                         \
    uint64_t index   = hash & mask;                                     \
    T##Kv* tombstone = NULL;                                            \
                                                                        \
    for (;;) {                                                          \
      T##Kv* kv = &kvs[index];                                          \
      if (kv->key == noK) {                                             \
        if (kv->val == noV) {                                           \
          return tombstone != NULL ? tombstone : kv;                    \
        } else {                                                        \
          if (tombstone == NULL)                                        \
            tombstone = kv;                                             \
        }                                                               \
      } else if (cmp(kv->key, k)) {                                     \
        return kv;                                                      \
      } else {                                                          \
        index = (index + 1) & mask;                                     \
      }                                                                 \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void adjust##T##Capacity(T* t, size_t cap) {                   \
    size_t newSize = cap * sizeof(T##Kv);                               \
    T##Kv* kvs     = allocate(NULL, newSize);                           \
    /* fill table */                                                    \
    for (size_t i=0; i<cap; i++)                                        \
      kvs[i] = (T##Kv) { .key=noK, .val=noV };                          \
                                                                        \
    /* reset table init count */                                        \
    t->cnt = 0;                                                         \
    if (t->kvs != NULL) {                                             \
      for (size_t i=0; i < t->cap; i++) {                               \
        T##Kv* kv = &t->kvs[i];                                         \
                                                                        \
        if (kv->key == noK)                                            \
          continue;                                                     \
                                                                        \
        T##Kv* dest = find##T##Kv(kvs, cap, kv->key);                   \
        dest->key   = kv->key;                                          \
        dest->val   = kv->val;                                          \
        t->cnt += 1;                                                \
      }                                                                 \
      deallocate(NULL, t->kvs, 0);                                     \
    }                                                                   \
    t->kvs = kvs;                                                       \
    t->cap = cap;                                                       \
  }                                                                     \
                                                                        \
  bool _t##Intern(T* t, K k, funcptr_t i, void* s, void* b) {           \
    if (t->cnt + 1 > t->cap * TABLE_MAX_LOAD)                           \
      adjust##T##Capacity(t, max(8u, t->cap << 1));                     \
                                                                        \
    T##Kv* kv   = find##T##Kv(t->kvs, t->cap, k);                       \
    bool isNewK = kv->key == noK;                                       \
                                                                        \
    if (isNewK) {                                                       \
      if (kv->val == noV)                                               \
        t->cnt++;                                                       \
                                                                        \
      void (*intern)(T* t, K k, void* s, void* b) = (void(*)())i;       \
      intern(t, k, s, b);                                               \
    }                                                                   \
                                                                        \
    return isNewK;                                                      \
  }                                                                     \
                                                                        \
  bool _t##Add(T* t, K k, V v) {                                        \
    if (t->cnt + 1 > t->cap * TABLE_MAX_LOAD)                           \
      adjust##T##Capacity(t, max(8u, t->cap << 1));                     \
                                                                        \
    T##Kv* kv   = find##T##Kv(t->kvs, t->cap, k);                       \
    bool isNewK = kv->key == noK;                                       \
                                                                        \
    if (isNewK) {                                                       \
      if (kv->val == noV)                                               \
        t->cnt++;                                                       \
      kv->key = k;                                                      \
      kv->val = v;                                                      \
    }                                                                   \
                                                                        \
    return isNewK;                                                      \
  }                                                                     \
                                                                        \
  bool _t##Get(T* t, K k, V* v) {                                       \
    bool out = false;                                                   \
    if (v != NULL)                                                      \
      *v = noV;                                                         \
                                                                        \
    if (t->cnt > 0) {                                                   \
      T##Kv* kv = find##T##Kv(t->kvs, t->cap, k);                       \
                                                                        \
      if (kv->key != noK) {                                             \
        out = true;                                                     \
        if (v != NULL)                                                  \
          *v = kv->val;                                                 \
      }                                                                 \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##Has(T* t, K k) {                                             \
    return _t##Get(t, k, NULL);                                         \
  }                                                                     \
                                                                        \
  bool _t##Set(T* t, K k, V v) {                                        \
    if (t->cnt + 1 > t->cap * TABLE_MAX_LOAD)                           \
      adjust##T##Capacity(t, max(8u, t->cap << 1));                     \
                                                                        \
    T##Kv* kv = find##T##Kv(t->kvs, t->cap, k);                         \
                                                                        \
    bool isNewK = kv->key == noK;                                       \
                                                                        \
    if (isNewK && kv->val == noV)                                       \
      t->cnt++;                                                         \
                                                                        \
    kv->key = k;                                                        \
    kv->val = v;                                                        \
                                                                        \
    return isNewK;                                                      \
  }                                                                     \
                                                                        \
  bool _t##Del(T* t, K k) {                                             \
    if (t->cnt == 0)                                                    \
      return false;                                                     \
                                                                        \
    T##Kv* kv = find##T##Kv(t->kvs, t->cap, k);                         \
                                                                        \
    if (kv->key == noK)                                                 \
      return false;                                                     \
                                                                        \
    kv->key = noK;                                                      \
    return true;                                                        \
  }

#define TABLE_OBJ_API(T, K, V, _t, cmp, hashK, noK, noV)                \
  T* new##T(int fl) {                                                   \
    T* t = newObj(&T##Type, fl, 0);                                     \
    init##T(t);                                                         \
    return t;                                                           \
  }                                                                     \
                                                                        \
  void init##T(T* t) {                                                  \
    t->kvs   = NULL;                                                    \
    t->cnt   = 0;                                                       \
    t->cap   = 0;                                                       \
  }                                                                     \
                                                                        \
  void free##T(T* t) {                                                  \
    deallocate(NULL, t->kvs, 0);                                        \
    init##T(t);                                                         \
  }                                                                     \
                                                                        \
  static T##Kv* find##T##Kv(T##Kv* kvs, size_t cap, K k) {              \
    uint64_t hash    = hashK(k);                                        \
    uint64_t mask    = cap - 1;                                         \
    uint64_t index   = hash & mask;                                     \
    T##Kv* tombstone = NULL;                                            \
                                                                        \
    for (;;) {                                                          \
      T##Kv* kv = &kvs[index];                                          \
      if (kv->key == noK) {                                             \
        if (kv->val == noV) {                                           \
          return tombstone != NULL ? tombstone : kv;                    \
        } else {                                                        \
          if (tombstone == NULL)                                        \
            tombstone = kv;                                             \
        }                                                               \
      } else if (cmp(kv->key, k)) {                                     \
        return kv;                                                      \
      } else {                                                          \
        index = (index + 1) & mask;                                     \
      }                                                                 \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void adjust##T##Capacity(T* t, size_t cap) {                   \
    size_t newSize = cap * sizeof(T##Kv);                               \
    T##Kv* kvs     = allocate(NULL, newSize);                           \
    /* fill table */                                                    \
    for (size_t i=0; i<cap; i++)                                        \
      kvs[i] = (T##Kv) { .key=noK, .val=noV };                          \
                                                                        \
    /* reset table init count */                                        \
    t->cnt = 0;                                                         \
    if (t->kvs != NULL) {                                               \
      for (size_t i=0; i < t->cap; i++) {                               \
        T##Kv* kv = &t->kvs[i];                                         \
                                                                        \
        if (kv->key == noK)                                             \
          continue;                                                     \
                                                                        \
        T##Kv* dest = find##T##Kv(kvs, cap, kv->key);                   \
        dest->key   = kv->key;                                          \
        dest->val   = kv->val;                                          \
        t->cnt += 1;                                                    \
      }                                                                 \
      deallocate(NULL, t->kvs, 0);                                      \
    }                                                                   \
    t->kvs = kvs;                                                       \
    t->cap = cap;                                                       \
  }                                                                     \
                                                                        \
  bool _t##Intern(T* t, K k, funcptr_t i, void* s, void* b) {           \
    if (t->cnt + 1 > t->cap * TABLE_MAX_LOAD)                           \
      adjust##T##Capacity(t, max(8u, t->cap << 1));                     \
                                                                        \
    T##Kv* kv   = find##T##Kv(t->kvs, t->cap, k);                       \
    bool isNewK = kv->key == noK;                                       \
                                                                        \
    if (isNewK) {                                                       \
      if (kv->val == noV)                                               \
        t->cnt++;                                                       \
                                                                        \
      void (*intern)(T* t, K k, void* s, void* b) = (void(*)())i;       \
      intern(t, k, s, b);                                               \
    }                                                                   \
                                                                        \
    return isNewK;                                                      \
  }                                                                     \
                                                                        \
  bool _t##Add(T* t, K k, V v) {                                        \
    if (t->cnt + 1 > t->cap * TABLE_MAX_LOAD)                           \
      adjust##T##Capacity(t, max(8u, t->cap << 1));                     \
                                                                        \
    T##Kv* kv   = find##T##Kv(t->kvs, t->cap, k);                       \
    bool isNewK = kv->key == noK;                                       \
                                                                        \
    if (isNewK) {                                                       \
      if (kv->val == noV)                                               \
        t->cnt++;                                                       \
      kv->key = k;                                                      \
      kv->val = v;                                                      \
    }                                                                   \
                                                                        \
    return isNewK;                                                      \
  }                                                                     \
                                                                        \
  bool _t##Get(T* t, K k, V* v) {                                       \
    bool out = false;                                                   \
    if (v != NULL)                                                      \
      *v = noV;                                                         \
                                                                        \
    if (t->cnt > 0) {                                                   \
      T##Kv* kv = find##T##Kv(t->kvs, t->cap, k);                       \
                                                                        \
      if (kv->key != noK) {                                             \
        out = true;                                                     \
        if (v != NULL)                                                  \
          *v = kv->val;                                                 \
      }                                                                 \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##Has(T* t, K k) {                                             \
    return _t##Get(t, k, NULL);                                         \
  }                                                                     \
                                                                        \
  bool _t##Set(T* t, K k, V v) {                                        \
    if (t->cnt + 1 > t->cap * TABLE_MAX_LOAD)                           \
      adjust##T##Capacity(t, max(8u, t->cap << 1));                     \
                                                                        \
    T##Kv* kv = find##T##Kv(t->kvs, t->cap, k);                         \
                                                                        \
    bool isNewK = kv->key == noK;                                       \
                                                                        \
    if (isNewK && kv->val == noV)                                       \
      t->cnt++;                                                     \
                                                                        \
    kv->key = k;                                                        \
    kv->val = v;                                                        \
                                                                        \
    return isNewK;                                                      \
  }                                                                     \
                                                                        \
  bool _t##Del(T* t, K k) {                                             \
    if (t->cnt == 0)                                                    \
      return false;                                                     \
                                                                        \
    T##Kv* kv = find##T##Kv(t->kvs, t->cap, k);                         \
                                                                        \
    if (kv->key == noK)                                                 \
      return false;                                                     \
                                                                        \
    kv->key = noK;                                                      \
    return true;                                                        \
  }


#endif
