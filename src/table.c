#include "util/number.h"
#include "util/collection.h"
#include "util/hashing.h"

#include "equal.h"

#include "memory.h"
#include "environment.h"
#include "type.h"
#include "table.h"

#undef TABLE_API
#undef ORDERED_TABLE_API

#define TABLE_API(T, K, V, _t, _i, _h, _e, noK, noV)                    \
  T* new##T(flags_t fl, T##InternFn i, T##HashFn h, T##EgalFn e) {      \
    T* t = newObj(&T##Type, fl, 0);                                     \
    init##T(t);                                                         \
    t->intern = i ? i : _i;                                             \
    t->hash   = h ? h : _h;                                             \
    t->egal   = e ? e : _e;                                             \
    return t;                                                           \
  }                                                                     \
                                                                        \
  void init##T(T* t) {                                                  \
    t->kvs = NULL;                                                      \
    t->cnt = 0;                                                         \
    t->cap = 0;                                                         \
  }                                                                     \
                                                                        \
  void free##T(void* p) {                                               \
    T* t = p;                                                           \
    deallocate(NULL, t->kvs, 0);                                        \
    init##T(t);                                                         \
  }                                                                     \
                                                                        \
  T##Kv* _t##Find(T* t, T##Kv* kvs, size_t c, K k) {                    \
    hash_t h  = t->hash(k);                                             \
    size_t m  = c - 1;                                                  \
    size_t i  = h & m;                                                  \
    T##Kv* ts = NULL;                                                   \
                                                                        \
    for (;;) {                                                          \
      T##Kv* kv = &kvs[i];                                              \
                                                                        \
      if (kv->key == noK) {                                             \
        if (kv->val == noV) {                                           \
          return ts ? ts : kv;                                          \
        } else if (ts == NULL) {                                        \
          ts = kv;                                                      \
        }                                                               \
      } else if (t->egal(kv->key, k)) {                                 \
        return kv;                                                      \
      } else {                                                          \
        i = (i + 1) & m;                                                \
      }                                                                 \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void adjust##T##Cap(T* t, size_t newCnt) {                     \
    if (newCnt == 0) {                                                  \
      free##T(t);                                                       \
    } else if (checkTableResize(newCnt, t->cap)) {                      \
      size_t oldCnt  = t->cnt;                                          \
      size_t oldCap  = t->cap;                                          \
      size_t newCap  = padOrdSize(oldCnt, newCnt, oldCap);              \
      size_t newSize = newCap * sizeof(T##Kv);                          \
      T##Kv* kvs    = allocate(NULL, newSize);                          \
                                                                        \
      /* initialize kvs */                                              \
      for (size_t i=0; i<newCap; i++)                                   \
        kvs[i] = (T##Kv) { noK, noV };                                  \
                                                                        \
      /* rehash */                                                      \
      if (t->kvs != NULL) {                                             \
        t->cnt = 0;                                                     \
        for (size_t i=0; i<oldCap; i++) {                               \
          T##Kv* src = &t->kvs[i];                                      \
                                                                        \
          if (src->key == noK)                                          \
            continue;                                                   \
                                                                        \
          T##Kv* dst = _t##Find(t, kvs, newCap, src->key);              \
          dst->key   = src->key;                                        \
          dst->val   = src->val;                                        \
          t->cnt++;                                                     \
        }                                                               \
        deallocate(NULL, t->kvs, 0);                                    \
      }                                                                 \
      t->kvs = kvs;                                                     \
      t->cap = newCap;                                                  \
    }                                                                   \
  }                                                                     \
                                                                        \
  bool _t##Intern(T* t, K k, void* s, V* b) {                           \
    adjust##T##Cap(t, t->cnt+1);                                        \
    T##Kv* dst = _t##Find(t, t->kvs, t->cap, k);                        \
    bool out   = dst->key == noK;                                       \
                                                                        \
    if (out) {                                                          \
      if (dst->val == noV)                                              \
        t->cnt++;                                                       \
      t->intern(t, dst, k, s, b);                                       \
    }                                                                   \
                                                                        \
    if (b != NULL)                                                      \
      *b = dst->val;                                                    \
                                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##Add(T* t, K k, V* v) {                                       \
    adjust##T##Cap(t, t->cnt+1);                                        \
    T##Kv* dst = _t##Find(t, t->kvs, t->cap, k);                        \
    bool out   = dst->key == noK;                                       \
                                                                        \
    if (out) {                                                          \
      if (dst->val == noV)                                              \
        t->cnt++;                                                       \
      dst->key = k;                                                     \
      dst->val = v ? *v : noV;                                          \
    } else if (v != NULL) {                                             \
      *v = dst->val;                                                    \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##Get(T* t, K k, V* v) {                                       \
    T##Kv* dst = _t##Find(t, t->kvs, t->cap, k);                        \
    bool   out = dst->key == noK;                                       \
                                                                        \
    if (!out && v != NULL)                                              \
      *v = dst->val;                                                    \
                                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##Set(T* t, K k, V v) {                                        \
    adjust##T##Cap(t, t->cnt+1);                                        \
    T##Kv* dst = _t##Find(t, t->kvs, t->cap, k);                        \
    bool   out = dst->key == noK;                                       \
                                                                        \
    if (out) {                                                          \
      t->cnt  += dst->val == noV;                                       \
      dst->key = k;                                                     \
    }                                                                   \
    dst->val = v;                                                       \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##Has(T* t, K k) {                                             \
    return _t##Get(t, k, NULL);                                         \
  }                                                                     \
                                                                        \
  bool _t##Del(T* t, K k) {                                             \
    T##Kv* kv  = _t##Find(t, t->kvs, t->cap, k);                        \
    bool   out = kv->key != noK;                                        \
    kv->key    = noK;                                                   \
                                                                        \
    return out;                                                         \
  }

#define ORDERED_TABLE_API(T, K, V, _t, _i, _h, _e, noK, noV)            \
  T* new##T(flags_t fl, T##InternFn i, T##HashFn h, T##EgalFn e) {      \
    T* t = newObj(&T##Type, fl, 0);                                     \
    init##T(t);                                                         \
    t->intern = i ? i : _i;                                             \
    t->hash   = h ? h : _h;                                             \
    t->egal   = e ? e : _e;                                             \
    return t;                                                           \
  }                                                                     \
                                                                        \
  void init##T(T* t) {                                                  \
    t->kvs    = NULL;                                                   \
    t->ord    = NULL;                                                   \
    t->kvCnt  = 0;                                                      \
    t->tsCnt  = 0;                                                      \
    t->kvCap  = 0;                                                      \
    t->ordCap = 0;                                                      \
  }                                                                     \
                                                                        \
  void free##T(void* p) {                                               \
    T* t = p;                                                           \
    deallocate(NULL, t->kvs, 0);                                        \
    deallocate(NULL, t->ord, 0);                                        \
    init##T(t);                                                         \
  }                                                                     \
                                                                        \
  size_t _t##Arity(T* t) {                                              \
    return t->kvCnt - t->tsCnt;                                         \
  }                                                                     \
                                                                        \
  void*  _t##Find(T* t, T##Kv* kvs, void* o, size_t c, K k) {           \
    hash_t h         = t->hash(k);                                      \
    size_t m         = c - 1;                                           \
    size_t i         = h & m;                                           \
    size_t os        = getOrdSize(c);                                   \
                                                                        \
    if (os == 1) {                                                      \
      int8_t o8, *o8s = o;                                              \
                                                                        \
      for (o8=o8s[i]; o8 != -1; i=(i + 1) & m, o8=o8s[i]) {             \
        if (o8 == -2)                                                   \
          continue;                                                     \
                                                                        \
        if (t->egal(kvs[o8].key, k))                                    \
          break;                                                        \
      }                                                                 \
    } else if (os == 2) {                                               \
      int16_t o16, *o16s = o;                                           \
                                                                        \
      for (o16=o16s[i]; o16 != -1; i=(i + 1) & m, o16=o16s[i]) {        \
        if (o16 == -2)                                                  \
          continue;                                                     \
                                                                        \
        if (t->egal(kvs[o16].key, k))                                   \
          break;                                                        \
      }                                                                 \
    } else if (os == 4) {                                               \
      int32_t o32, *o32s = o;                                           \
                                                                        \
      for (o32=o32s[i]; o32 != -1; i=(i + 1) & m, o32=o32s[i]) {        \
        if (o32 == -2)                                                  \
          continue;                                                     \
                                                                        \
        if (t->egal(kvs[o32].key, k))                                   \
          break;                                                        \
      }                                                                 \
    }  else {                                                           \
      assert(os == 8);                                                  \
      int64_t o64, *o64s = o;                                           \
                                                                        \
      for (o64=o64s[i]; o64 != -1; i=(i + 1) & m, o64=o64s[i]) {        \
        if (o64 == -2)                                                  \
          continue;                                                     \
                                                                        \
        if (t->egal(kvs[o64].key, k))                                   \
          break;                                                        \
      }                                                                 \
    }                                                                   \
    return o + (i*os);                                                  \
  }                                                                     \
                                                                        \
  static void adjust##T##Cap(T* t, size_t cnt) {                        \
    if (cnt == 0) {                                                     \
      free##T(t);                                                       \
    } else if (checkTableResize(cnt, t->ordCap)) {                      \
      size_t oldKv  = t->kvCnt;                                         \
      size_t newCnt = cnt - t->tsCnt;                                   \
      size_t kvCap  = padAlistSize(cnt, newCnt, t->kvCap, false);       \
      size_t ordCap = padOrdSize(cnt, newCnt, t->ordCap);               \
      T##Kv* kvs    = allocate(NULL, kvCap * sizeof(T##Kv));            \
      void*  ords   = allocOrds(ordCap, 0);                             \
                                                                        \
      /* initalize key/value pairs */                                   \
      for (size_t i=0; i<kvCap; i++)                                    \
        kvs[i] = (T##Kv) { noK, noV };                                  \
                                                                        \
      t->tsCnt = 0;                                                     \
      t->kvCnt = 0;                                                     \
      if (t->ord != NULL) {                                             \
        if (ordCap <= INT8_MAX) {                                       \
          for (size_t i=0; i < oldKv; i++) {                            \
            if (t->kvs[i].key == noK)                                   \
              continue;                                                 \
            T##Kv* src = &t->kvs[i];                                    \
            T##Kv* dst = &kvs[t->kvCnt];                                \
            int8_t* o8 = _t##Find(t, kvs, ords, ordCap, src->key);      \
            dst->key   = src->key;                                      \
            dst->val   = src->val;                                      \
            *o8        = t->kvCnt++;                                    \
          }                                                             \
        } else if (ordCap <= INT16_MAX) {                               \
          for (size_t i=0; i < oldKv; i++) {                            \
            if (t->kvs[i].key == noK)                                   \
              continue;                                                 \
            T##Kv* src   = &t->kvs[i];                                  \
            T##Kv* dst   = &kvs[t->kvCnt];                              \
            int16_t* o16 = _t##Find(t, kvs, ords, ordCap, src->key);    \
            dst->key     = src->key;                                    \
            dst->val     = src->val;                                    \
            *o16         = t->kvCnt++;                                  \
          }                                                             \
        } else if (ordCap <= INT32_MAX) {                               \
          for (size_t i=0; i < oldKv; i++) {                            \
            if (t->kvs[i].key == noK)                                   \
              continue;                                                 \
            T##Kv* src   = &t->kvs[i];                                  \
            T##Kv* dst   = &kvs[t->kvCnt];                              \
            int32_t* o32 = _t##Find(t, kvs, ords, ordCap, src->key);   \
            dst->key     = src->key;                                    \
            dst->val     = src->val;                                    \
            *o32         = t->kvCnt++;                                  \
          }                                                             \
        } else {                                                        \
          for (size_t i=0; i < oldKv; i++) {                            \
            if (t->kvs[i].key == noK)                                   \
              continue;                                                 \
            T##Kv* src   = &t->kvs[i];                                  \
            T##Kv* dst   = &kvs[t->kvCnt];                              \
            int64_t* o64 = _t##Find(t, kvs, ords, ordCap, src->key);   \
            dst->key     = src->key;                                    \
            dst->val     = src->val;                                    \
            *o64         = t->kvCnt++;                                  \
          }                                                             \
        }                                                               \
      }                                                                 \
      deallocate(NULL, t->kvs, 0);                                      \
      deallocate(NULL, t->ord, 0);                                      \
      t->kvs    = kvs;                                                  \
      t->ord    = ords;                                                 \
      t->kvCap  = kvCap;                                                \
      t->ordCap = ordCap;                                               \
    } else if (checkAlistResize(cnt, t->kvCap, false)) {                \
      size_t newKvC  = padAlistSize(cnt, t->kvCnt, t->kvCap, false);    \
      size_t newKvS = newKvC * sizeof(T##Kv);                           \
      size_t oldKvS = t->kvCnt * sizeof(T##Kv);                         \
      t->kvs        = reallocate(NULL, t->kvs, oldKvS, newKvS);         \
      for (size_t i=t->kvCnt; i < t->kvCnt; i++)                        \
        t->kvs[i] = (T##Kv) { noK, noV };                               \
    }                                                                   \
  }                                                                     \
                                                                        \
  bool _t##Intern(T* t, K k, void* s, V* b) {                           \
    bool out;                                                           \
    long ord;                                                           \
    void* ords;                                                         \
    adjust##T##Cap(t, t->kvCnt+1);                                      \
    ords = _t##Find(t, t->kvs, t->ord, t->ordCap, k);                   \
    if (t->ordCap <= INT8_MAX) {                                        \
      ord = *(int8_t*)ords;                                             \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        ord = *(int8_t*)ords = t->kvCnt;                                \
        t->intern(t, &t->kvs[t->kvCnt], k, s, b);                       \
        t->kvCnt++;                                                     \
      }                                                                 \
    } else if (t->ordCap <= INT16_MAX) {                                \
      ord = *(int16_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        ord = *(int16_t*)ords = t->kvCnt;                               \
        t->intern(t, &t->kvs[t->kvCnt], k, s, b);                       \
        t->kvCnt++;                                                     \
      }                                                                 \
    } else if (t->ordCap <= INT32_MAX) {                                \
      ord = *(int32_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        ord = *(int32_t*)ords = t->kvCnt;                               \
        t->intern(t, &t->kvs[t->kvCnt], k, s, b);                       \
        t->kvCnt++;                                                     \
      }                                                                 \
    } else {                                                            \
      ord = *(int64_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        ord = *(int64_t*)ords = t->kvCnt;                               \
        t->intern(t, &t->kvs[t->kvCnt], k, s, b);                       \
        t->kvCnt++;                                                     \
      }                                                                 \
    }                                                                   \
    if (b != NULL)                                                      \
      *b = t->kvs[ord].val;                                             \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##Add(T* t, K k, V* v) {                                       \
    bool out;                                                           \
    long ord;                                                           \
    void* ords;                                                         \
    adjust##T##Cap(t, t->kvCnt+1);                                      \
    ords = _t##Find(t, t->kvs, t->ord, t->ordCap, k);                   \
    if (t->ordCap <= INT8_MAX) {                                        \
      ord = *(int8_t*)ords;                                             \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        *(int8_t*)ords = t->kvCnt;                                      \
        t->kvs[t->kvCnt++] = (T##Kv) { k, v ? *v : noV };               \
      } else if (v) {                                                   \
        *v = t->kvs[ord].val;                                           \
      }                                                                 \
    } else if (t->ordCap <= INT16_MAX) {                                \
      ord = *(int16_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out)  {                                                       \
        *(int16_t*)ords    = t->kvCnt;                                  \
        t->kvs[t->kvCnt++] = (T##Kv) { k, v ? *v : noV  };              \
      } else if (v) {                                                   \
        *v = t->kvs[ord].val;                                           \
      }                                                                 \
    } else if (t->ordCap <= INT32_MAX) {                                \
      ord = *(int32_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        *(int32_t*)ords = t->kvCnt;                                     \
        t->kvs[t->kvCnt++] = (T##Kv) { k, v ? *v : noV };               \
      } else if (v) {                                                   \
        *v = t->kvs[ord].val;                                           \
      }                                                                 \
    } else {                                                            \
      ord = *(int64_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        *(int64_t*)ords = t->kvCnt;                                     \
        t->kvs[t->kvCnt++] = (T##Kv) { k, v ? *v : noV };               \
      } else if (v) {                                                   \
        *v = t->kvs[ord].val;                                           \
      }                                                                 \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##Get(T* t, K k, V* v) {                                       \
    if (t->kvCnt == 0)                                                  \
      return false;                                                     \
                                                                        \
    bool out;                                                           \
    int64_t ord;                                                        \
    void* ords = _t##Find(t, t->kvs, t->ord, t->ordCap, k);             \
                                                                        \
    if (t->ordCap <= INT8_MAX)                                          \
      ord = *(int8_t*)ords;                                             \
    else if (t->ordCap <= INT16_MAX)                                    \
      ord = *(int16_t*)ords;                                            \
    else if (t->ordCap <= INT32_MAX)                                    \
      ord = *(int32_t*)ords;                                            \
    else                                                                \
      ord = *(int64_t*)ords;                                            \
                                                                        \
    out = ord < 0;                                                      \
                                                                        \
    if (out && v)                                                       \
      *v = t->kvs[ord].val;                                             \
                                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##Set(T* t, K k, V v) {                                        \
    bool out;                                                           \
    long ord;                                                           \
    void* ords;                                                         \
    adjust##T##Cap(t, t->kvCnt+1);                                      \
    ords = _t##Find(t, t->kvs, t->ord, t->ordCap, k);                   \
                                                                        \
    if (t->ordCap <= INT8_MAX) {                                        \
      ord = *(int8_t*)ords;                                             \
      out = ord < 0;                                                    \
      if (out) {                                                        \
        *(int8_t*)ords = t->kvCnt;                                      \
        t->kvs[t->kvCnt++] = (T##Kv) { k, v };                          \
      } else {                                                          \
        t->kvs[ord].val = v;                                            \
      }                                                                 \
    } else if (t->ordCap <= INT16_MAX) {                                \
      ord = *(int16_t*)ords;                                            \
      out = ord < 0;                                                    \
      if (out) {                                                        \
        *(int16_t*)ords = t->kvCnt;                                     \
        t->kvs[t->kvCnt++] = (T##Kv) { k, v };                          \
      } else {                                                          \
        t->kvs[ord].val = v;                                            \
      }                                                                 \
    } else if (t->ordCap <= INT32_MAX) {                                \
      ord = *(int32_t*)ords;                                            \
      out = ord < 0;                                                    \
      if (out) {                                                        \
        *(int32_t*)ords = t->kvCnt;                                     \
        t->kvs[t->kvCnt++] = (T##Kv) { k, v };                          \
      } else {                                                          \
        t->kvs[ord].val = v;                                            \
      }                                                                 \
    } else {                                                            \
      ord = *(int64_t*)ords;                                            \
      out = ord < 0;                                                    \
      if (out) {                                                        \
        *(int64_t*)ords = t->kvCnt;                                     \
        t->kvs[t->kvCnt++] = (T##Kv) { k, v };                          \
      } else {                                                          \
        t->kvs[ord].val = v;                                            \
      }                                                                 \
    }                                                                   \
                                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##Has(T* t, K k) {                                             \
    return _t##Get(t, k, NULL);                                         \
  }                                                                     \
                                                                        \
  bool _t##Del(T* t, K k) {                                             \
    bool  out;                                                          \
    long  ord;                                                          \
    void* ords = _t##Find(t, t->kvs, t->ord, t->ordCap, k);             \
                                                                        \
    if (t->ordCap <= INT8_MAX) {                                        \
      ord = *(int8_t*)ords;                                             \
      out = ord >= 0;                                                   \
      if (out) {                                                        \
        t->tsCnt++;                                                     \
        t->kvs[ord].key = noK;                                          \
        *(int8_t*)ords  = -2;                                           \
      }                                                                 \
    }  else if (t->ordCap <= INT16_MAX) {                               \
      ord = *(int16_t*)ords;                                            \
      out = ord >= 0;                                                   \
      if (out) {                                                        \
        t->tsCnt++;                                                     \
        t->kvs[ord].key = noK;                                          \
        *(int16_t*)ords = -2;                                           \
      }                                                                 \
    } else if (t->ordCap <= INT32_MAX) {                                \
      ord = *(int32_t*)ords;                                            \
      out = ord >= 0;                                                   \
      if (out) {                                                        \
        t->tsCnt++;                                                     \
        t->kvs[ord].key = noK;                                          \
        *(int32_t*)ords = -2;                                           \
      }                                                                 \
    } else {                                                            \
      ord = *(int64_t*)ords;                                            \
      out = ord >= 0;                                                   \
      if (out) {                                                        \
        t->tsCnt++;                                                     \
        t->kvs[ord].key = noK;                                          \
        *(int64_t*)ords = -2;                                           \
      }                                                                 \
    }                                                                   \
                                                                        \
    return out;                                                         \
  }

// Table type
void tIntern(Table* t, TableKv* kv, Value k, void* s, Value* v) {
  (void)t;
  (void)s;
  
  kv->key = k;
  kv->val = v ? *v : NUL;
}

TABLE_API(Table, Value, Value, table, tIntern, hash, equal, NOTHING, NOTHING);

// SymbolTable type 
bool stCmp(char* x, char* y) {
  return strcmp(x, y) == 0;
}

hash_t stHash(char* s) {
  return hashString(s);
}

void stIntern(SymbolTable* t, SymbolTableKv* kv, char* n, void* s, Symbol** b) {
  (void)t;
  (void)s;
  (void)b;

  kv->val = newSymbol(n, 0);
  kv->key = kv->val->name;
}

TABLE_API(SymbolTable, char*, Symbol*, symbolTable, stIntern, stHash, stCmp, NULL, NULL);

// NameSpace type
bool nsCmp(Symbol* x, Symbol* y) {
  return x == y;
}

hash_t nsHash(Symbol* k) {
  return hashPtr(k);
}

void nsIntern(NameSpace* ns, NameSpaceKv* kv, Symbol* n, void* s, Binding** b) {
  (void)s;
  (void)b;

  NsType type = getNsType(ns);

  Value ini;
  Binding* captured = b && *b ? *b : NULL;

  if (type == GLOBAL_NS || type == PRIVATE_NS)
    ini = s ? *(Value*)s : NUL;

  else
    ini = NOTHING;

  kv->key = n;
  kv->val = newBinding(captured, n, ns, ns->kvCnt, 0, ini);
}

ORDERED_TABLE_API(NameSpace, Symbol*, Binding*, namespace, nsIntern, nsHash, nsCmp, NULL, NULL);

NsType getNsType(NameSpace* ns) {
  assert(ns);

  return ns->obj.flags & 0x7;
}
