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

#define TABLE_API(T, K, V, _t, _i, _h, _e, no_k, no_v)                    \
  T* new_##_t(flags_t fl, T##InternFn i, T##HashFn h, T##EgalFn e) {      \
    T* t = new_obj(&T##Type, fl, 0);                                     \
    init_##_t(t);                                                         \
    t->intern = i ? i : _i;                                             \
    t->hash   = h ? h : _h;                                             \
    t->egal   = e ? e : _e;                                             \
    return t;                                                           \
  }                                                                     \
                                                                        \
  void init_##_t(T* t) {                                                  \
    t->kvs = NULL;                                                      \
    t->cnt = 0;                                                         \
    t->cap = 0;                                                         \
  }                                                                     \
                                                                        \
  void free_##_t(void* p) {                                               \
    T* t = p;                                                           \
    deallocate(NULL, t->kvs, 0);                                        \
    init_##_t(t);                                                         \
  }                                                                     \
                                                                        \
  T##Kv* _t##_find(T* t, T##Kv* kvs, size_t c, K k) {                    \
    hash_t h  = t->hash(k);                                             \
    size_t m  = c - 1;                                                  \
    size_t i  = h & m;                                                  \
    T##Kv* ts = NULL;                                                   \
                                                                        \
    for (;;) {                                                          \
      T##Kv* kv = &kvs[i];                                              \
                                                                        \
      if (kv->key == no_k) {                                             \
        if (kv->val == no_v) {                                           \
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
  static void adjust_##_t##_cap(T* t, size_t new_cnt) {                 \
    if (new_cnt == 0) {                                                 \
      free_##_t(t);                                                     \
    } else if (check_table_resize(new_cnt, t->cap)) {                   \
      size_t old_cnt  = t->cnt;                                         \
      size_t old_cap  = t->cap;                                         \
      size_t new_cap  = pad_ord_size(old_cnt, new_cnt, old_cap);        \
      size_t new_size = new_cap * sizeof(T##Kv);                        \
      T##Kv* kvs    = allocate(NULL, new_size);                         \
                                                                        \
      /* initialize kvs */                                              \
      for (size_t i=0; i<new_cap; i++)                                  \
        kvs[i] = (T##Kv) { no_k, no_v };                                  \
                                                                        \
      /* rehash */                                                      \
      if (t->kvs != NULL) {                                             \
        t->cnt = 0;                                                     \
        for (size_t i=0; i<old_cap; i++) {                               \
          T##Kv* src = &t->kvs[i];                                      \
                                                                        \
          if (src->key == no_k)                                          \
            continue;                                                   \
                                                                        \
          T##Kv* dst = _t##_find(t, kvs, new_cap, src->key);              \
          dst->key   = src->key;                                        \
          dst->val   = src->val;                                        \
          t->cnt++;                                                     \
        }                                                               \
        deallocate(NULL, t->kvs, 0);                                    \
      }                                                                 \
      t->kvs = kvs;                                                     \
      t->cap = new_cap;                                                  \
    }                                                                   \
  }                                                                     \
                                                                        \
  bool _t##_intern(T* t, K k, void* s, V* b) {                           \
    adjust_##_t##_cap(t, t->cnt+1);                                        \
    T##Kv* dst = _t##_find(t, t->kvs, t->cap, k);                        \
    bool out   = dst->key == no_k;                                       \
                                                                        \
    if (out) {                                                          \
      if (dst->val == no_v)                                              \
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
  bool _t##_add(T* t, K k, V* v) {                                       \
    adjust_##_t##_cap(t, t->cnt+1);                                        \
    T##Kv* dst = _t##_find(t, t->kvs, t->cap, k);                        \
    bool out   = dst->key == no_k;                                       \
                                                                        \
    if (out) {                                                          \
      if (dst->val == no_v)                                              \
        t->cnt++;                                                       \
      dst->key = k;                                                     \
      dst->val = v ? *v : no_v;                                          \
    } else if (v != NULL) {                                             \
      *v = dst->val;                                                    \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##_get(T* t, K k, V* v) {                                       \
    T##Kv* dst = _t##_find(t, t->kvs, t->cap, k);                        \
    bool   out = dst->key == no_k;                                       \
                                                                        \
    if (!out && v != NULL)                                              \
      *v = dst->val;                                                    \
                                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##_set(T* t, K k, V v) {                                        \
    adjust_##_t##_cap(t, t->cnt+1);                                        \
    T##Kv* dst = _t##_find(t, t->kvs, t->cap, k);                        \
    bool   out = dst->key == no_k;                                       \
                                                                        \
    if (out) {                                                          \
      t->cnt  += dst->val == no_v;                                       \
      dst->key = k;                                                     \
    }                                                                   \
    dst->val = v;                                                       \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##_has(T* t, K k) {                                             \
    return _t##_get(t, k, NULL);                                         \
  }                                                                     \
                                                                        \
  bool _t##Del(T* t, K k) {                                             \
    T##Kv* kv  = _t##_find(t, t->kvs, t->cap, k);                        \
    bool   out = kv->key != no_k;                                        \
    kv->key    = no_k;                                                   \
                                                                        \
    return out;                                                         \
  }

#define ORDERED_TABLE_API(T, K, V, _t, _i, _h, _e, no_k, no_v)            \
  T* new_##_t(flags_t fl, T##InternFn i, T##HashFn h, T##EgalFn e) {    \
    T* t = new_obj(&T##Type, fl, 0);                                    \
    init_##_t(t);                                                       \
    t->intern = i ? i : _i;                                             \
    t->hash   = h ? h : _h;                                             \
    t->egal   = e ? e : _e;                                             \
    return t;                                                           \
  }                                                                     \
                                                                        \
  void init_##_t(T* t) {                                                \
    t->kvs    = NULL;                                                   \
    t->ord    = NULL;                                                   \
    t->kv_cnt  = 0;                                                      \
    t->ts_cnt  = 0;                                                      \
    t->kv_cap  = 0;                                                      \
    t->ord_cap = 0;                                                      \
  }                                                                     \
                                                                        \
  void free_##_t(void* p) {                                             \
    T* t = p;                                                           \
    deallocate(NULL, t->kvs, 0);                                        \
    deallocate(NULL, t->ord, 0);                                        \
    init_##_t(t);                                                       \
  }                                                                     \
                                                                        \
  size_t _t##Arity(T* t) {                                              \
    return t->kv_cnt - t->ts_cnt;                                         \
  }                                                                     \
                                                                        \
  void*  _t##_find(T* t, T##Kv* kvs, void* o, size_t c, K k) {           \
    hash_t h         = t->hash(k);                                      \
    size_t m         = c - 1;                                           \
    size_t i         = h & m;                                           \
    size_t os        = get_ord_size(c);                                   \
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
  static void adjust_##_t##_cap(T* t, size_t cnt) {                        \
    if (cnt == 0) {                                                     \
      free_##_t(t);                                                       \
    } else if (check_table_resize(cnt, t->ord_cap)) {                      \
      size_t oldKv  = t->kv_cnt;                                         \
      size_t new_cnt = cnt - t->ts_cnt;                                   \
      size_t kv_cap  = pad_alist_size(cnt, new_cnt, t->kv_cap, false);       \
      size_t ord_cap = pad_ord_size(cnt, new_cnt, t->ord_cap);               \
      T##Kv* kvs    = allocate(NULL, kv_cap * sizeof(T##Kv));            \
      void*  ords   = alloc_ords(ord_cap, 0);                             \
                                                                        \
      /* initalize key/value pairs */                                   \
      for (size_t i=0; i<kv_cap; i++)                                    \
        kvs[i] = (T##Kv) { no_k, no_v };                                  \
                                                                        \
      t->ts_cnt = 0;                                                     \
      t->kv_cnt = 0;                                                     \
      if (t->ord != NULL) {                                             \
        if (ord_cap <= INT8_MAX) {                                       \
          for (size_t i=0; i < oldKv; i++) {                            \
            if (t->kvs[i].key == no_k)                                   \
              continue;                                                 \
            T##Kv* src = &t->kvs[i];                                    \
            T##Kv* dst = &kvs[t->kv_cnt];                                \
            int8_t* o8 = _t##_find(t, kvs, ords, ord_cap, src->key);      \
            dst->key   = src->key;                                      \
            dst->val   = src->val;                                      \
            *o8        = t->kv_cnt++;                                    \
          }                                                             \
        } else if (ord_cap <= INT16_MAX) {                               \
          for (size_t i=0; i < oldKv; i++) {                            \
            if (t->kvs[i].key == no_k)                                   \
              continue;                                                 \
            T##Kv* src   = &t->kvs[i];                                  \
            T##Kv* dst   = &kvs[t->kv_cnt];                              \
            int16_t* o16 = _t##_find(t, kvs, ords, ord_cap, src->key);    \
            dst->key     = src->key;                                    \
            dst->val     = src->val;                                    \
            *o16         = t->kv_cnt++;                                  \
          }                                                             \
        } else if (ord_cap <= INT32_MAX) {                               \
          for (size_t i=0; i < oldKv; i++) {                            \
            if (t->kvs[i].key == no_k)                                   \
              continue;                                                 \
            T##Kv* src   = &t->kvs[i];                                  \
            T##Kv* dst   = &kvs[t->kv_cnt];                              \
            int32_t* o32 = _t##_find(t, kvs, ords, ord_cap, src->key);   \
            dst->key     = src->key;                                    \
            dst->val     = src->val;                                    \
            *o32         = t->kv_cnt++;                                  \
          }                                                             \
        } else {                                                        \
          for (size_t i=0; i < oldKv; i++) {                            \
            if (t->kvs[i].key == no_k)                                   \
              continue;                                                 \
            T##Kv* src   = &t->kvs[i];                                  \
            T##Kv* dst   = &kvs[t->kv_cnt];                              \
            int64_t* o64 = _t##_find(t, kvs, ords, ord_cap, src->key);   \
            dst->key     = src->key;                                    \
            dst->val     = src->val;                                    \
            *o64         = t->kv_cnt++;                                  \
          }                                                             \
        }                                                               \
      }                                                                 \
      deallocate(NULL, t->kvs, 0);                                      \
      deallocate(NULL, t->ord, 0);                                      \
      t->kvs    = kvs;                                                  \
      t->ord    = ords;                                                 \
      t->kv_cap  = kv_cap;                                                \
      t->ord_cap = ord_cap;                                               \
    } else if (check_alist_resize(cnt, t->kv_cap, false)) {                \
      size_t newKvC  = pad_alist_size(cnt, t->kv_cnt, t->kv_cap, false);    \
      size_t newKvS = newKvC * sizeof(T##Kv);                           \
      size_t oldKvS = t->kv_cnt * sizeof(T##Kv);                         \
      t->kvs        = reallocate(NULL, t->kvs, oldKvS, newKvS);         \
      for (size_t i=t->kv_cnt; i < t->kv_cnt; i++)                        \
        t->kvs[i] = (T##Kv) { no_k, no_v };                               \
    }                                                                   \
  }                                                                     \
                                                                        \
  bool _t##_intern(T* t, K k, void* s, V* b) {                           \
    bool out;                                                           \
    long ord;                                                           \
    void* ords;                                                         \
    adjust_##_t##_cap(t, t->kv_cnt+1);                                      \
    ords = _t##_find(t, t->kvs, t->ord, t->ord_cap, k);                   \
    if (t->ord_cap <= INT8_MAX) {                                        \
      ord = *(int8_t*)ords;                                             \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        ord = *(int8_t*)ords = t->kv_cnt;                                \
        t->intern(t, &t->kvs[t->kv_cnt], k, s, b);                       \
        t->kv_cnt++;                                                     \
      }                                                                 \
    } else if (t->ord_cap <= INT16_MAX) {                                \
      ord = *(int16_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        ord = *(int16_t*)ords = t->kv_cnt;                               \
        t->intern(t, &t->kvs[t->kv_cnt], k, s, b);                       \
        t->kv_cnt++;                                                     \
      }                                                                 \
    } else if (t->ord_cap <= INT32_MAX) {                                \
      ord = *(int32_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        ord = *(int32_t*)ords = t->kv_cnt;                               \
        t->intern(t, &t->kvs[t->kv_cnt], k, s, b);                       \
        t->kv_cnt++;                                                     \
      }                                                                 \
    } else {                                                            \
      ord = *(int64_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        ord = *(int64_t*)ords = t->kv_cnt;                               \
        t->intern(t, &t->kvs[t->kv_cnt], k, s, b);                       \
        t->kv_cnt++;                                                     \
      }                                                                 \
    }                                                                   \
    if (b != NULL)                                                      \
      *b = t->kvs[ord].val;                                             \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##_add(T* t, K k, V* v) {                                       \
    bool out;                                                           \
    long ord;                                                           \
    void* ords;                                                         \
    adjust_##_t##_cap(t, t->kv_cnt+1);                                      \
    ords = _t##_find(t, t->kvs, t->ord, t->ord_cap, k);                   \
    if (t->ord_cap <= INT8_MAX) {                                        \
      ord = *(int8_t*)ords;                                             \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        *(int8_t*)ords = t->kv_cnt;                                      \
        t->kvs[t->kv_cnt++] = (T##Kv) { k, v ? *v : no_v };               \
      } else if (v) {                                                   \
        *v = t->kvs[ord].val;                                           \
      }                                                                 \
    } else if (t->ord_cap <= INT16_MAX) {                                \
      ord = *(int16_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out)  {                                                       \
        *(int16_t*)ords    = t->kv_cnt;                                  \
        t->kvs[t->kv_cnt++] = (T##Kv) { k, v ? *v : no_v  };              \
      } else if (v) {                                                   \
        *v = t->kvs[ord].val;                                           \
      }                                                                 \
    } else if (t->ord_cap <= INT32_MAX) {                                \
      ord = *(int32_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        *(int32_t*)ords = t->kv_cnt;                                     \
        t->kvs[t->kv_cnt++] = (T##Kv) { k, v ? *v : no_v };               \
      } else if (v) {                                                   \
        *v = t->kvs[ord].val;                                           \
      }                                                                 \
    } else {                                                            \
      ord = *(int64_t*)ords;                                            \
      out = ord == -1;                                                  \
      if (out) {                                                        \
        *(int64_t*)ords = t->kv_cnt;                                     \
        t->kvs[t->kv_cnt++] = (T##Kv) { k, v ? *v : no_v };               \
      } else if (v) {                                                   \
        *v = t->kvs[ord].val;                                           \
      }                                                                 \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##_get(T* t, K k, V* v) {                                       \
    if (t->kv_cnt == 0)                                                  \
      return false;                                                     \
                                                                        \
    bool out;                                                           \
    int64_t ord;                                                        \
    void* ords = _t##_find(t, t->kvs, t->ord, t->ord_cap, k);             \
                                                                        \
    if (t->ord_cap <= INT8_MAX)                                          \
      ord = *(int8_t*)ords;                                             \
    else if (t->ord_cap <= INT16_MAX)                                    \
      ord = *(int16_t*)ords;                                            \
    else if (t->ord_cap <= INT32_MAX)                                    \
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
  bool _t##_set(T* t, K k, V v) {                                        \
    bool out;                                                           \
    long ord;                                                           \
    void* ords;                                                         \
    adjust_##_t##_cap(t, t->kv_cnt+1);                                      \
    ords = _t##_find(t, t->kvs, t->ord, t->ord_cap, k);                   \
                                                                        \
    if (t->ord_cap <= INT8_MAX) {                                        \
      ord = *(int8_t*)ords;                                             \
      out = ord < 0;                                                    \
      if (out) {                                                        \
        *(int8_t*)ords = t->kv_cnt;                                      \
        t->kvs[t->kv_cnt++] = (T##Kv) { k, v };                          \
      } else {                                                          \
        t->kvs[ord].val = v;                                            \
      }                                                                 \
    } else if (t->ord_cap <= INT16_MAX) {                                \
      ord = *(int16_t*)ords;                                            \
      out = ord < 0;                                                    \
      if (out) {                                                        \
        *(int16_t*)ords = t->kv_cnt;                                     \
        t->kvs[t->kv_cnt++] = (T##Kv) { k, v };                          \
      } else {                                                          \
        t->kvs[ord].val = v;                                            \
      }                                                                 \
    } else if (t->ord_cap <= INT32_MAX) {                                \
      ord = *(int32_t*)ords;                                            \
      out = ord < 0;                                                    \
      if (out) {                                                        \
        *(int32_t*)ords = t->kv_cnt;                                     \
        t->kvs[t->kv_cnt++] = (T##Kv) { k, v };                          \
      } else {                                                          \
        t->kvs[ord].val = v;                                            \
      }                                                                 \
    } else {                                                            \
      ord = *(int64_t*)ords;                                            \
      out = ord < 0;                                                    \
      if (out) {                                                        \
        *(int64_t*)ords = t->kv_cnt;                                     \
        t->kvs[t->kv_cnt++] = (T##Kv) { k, v };                          \
      } else {                                                          \
        t->kvs[ord].val = v;                                            \
      }                                                                 \
    }                                                                   \
                                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool _t##_has(T* t, K k) {                                             \
    return _t##_get(t, k, NULL);                                         \
  }                                                                     \
                                                                        \
  bool _t##Del(T* t, K k) {                                             \
    bool  out;                                                          \
    long  ord;                                                          \
    void* ords = _t##_find(t, t->kvs, t->ord, t->ord_cap, k);             \
                                                                        \
    if (t->ord_cap <= INT8_MAX) {                                        \
      ord = *(int8_t*)ords;                                             \
      out = ord >= 0;                                                   \
      if (out) {                                                        \
        t->ts_cnt++;                                                     \
        t->kvs[ord].key = no_k;                                          \
        *(int8_t*)ords  = -2;                                           \
      }                                                                 \
    }  else if (t->ord_cap <= INT16_MAX) {                               \
      ord = *(int16_t*)ords;                                            \
      out = ord >= 0;                                                   \
      if (out) {                                                        \
        t->ts_cnt++;                                                     \
        t->kvs[ord].key = no_k;                                          \
        *(int16_t*)ords = -2;                                           \
      }                                                                 \
    } else if (t->ord_cap <= INT32_MAX) {                                \
      ord = *(int32_t*)ords;                                            \
      out = ord >= 0;                                                   \
      if (out) {                                                        \
        t->ts_cnt++;                                                     \
        t->kvs[ord].key = no_k;                                          \
        *(int32_t*)ords = -2;                                           \
      }                                                                 \
    } else {                                                            \
      ord = *(int64_t*)ords;                                            \
      out = ord >= 0;                                                   \
      if (out) {                                                        \
        t->ts_cnt++;                                                     \
        t->kvs[ord].key = no_k;                                          \
        *(int64_t*)ords = -2;                                           \
      }                                                                 \
    }                                                                   \
                                                                        \
    return out;                                                         \
  }

// Table type
void t_intern(Table* t, TableKv* kv, Value k, void* s, Value* v) {
  (void)t;
  (void)s;
  
  kv->key = k;
  kv->val = v ? *v : NUL;
}

TABLE_API(Table, Value, Value, table,
          t_intern, hash, equal,
          NOTHING, NOTHING);

// SymbolTable type 
bool st_cmp(char* x, char* y) {
  return strcmp(x, y) == 0;
}

hash_t st_hash(char* s) {
  return hash_string(s);
}

void st_intern(SymbolTable* t, SymbolTableKv* kv, char* n, void* s, Symbol** b) {
  (void)t;
  (void)s;
  (void)b;

  kv->val = new_symbol(n, 0);
  kv->key = kv->val->name;
}

TABLE_API(SymbolTable, char*, Symbol*, symbol_table,
          st_intern, st_hash, st_cmp,
          NULL, NULL);

// NameSpace type
bool ns_cmp(Symbol* x, Symbol* y) {
  return x == y;
}

hash_t ns_hash(Symbol* k) {
  return hash_ptr(k);
}

void ns_intern(NameSpace* ns, NameSpaceKv* kv, Symbol* n, void* s, Binding** b) {
  (void)s;
  (void)b;

  NsType type = get_ns_type(ns);

  Value ini;
  Binding* captured = b && *b ? *b : NULL;

  if (type == GLOBAL_NS || type == PRIVATE_NS)
    ini = s ? *(Value*)s : NUL;

  else
    ini = NOTHING;

  kv->key = n;
  kv->val = new_binding(captured, n, ns, ns->kv_cnt, 0, ini);
}

ORDERED_TABLE_API(NameSpace, Symbol*, Binding*, name_space,
                  ns_intern, ns_hash, ns_cmp,
                  NULL, NULL);

NsType get_ns_type(NameSpace* ns) {
  assert(ns);

  return ns->obj.flags & 0x7;
}
