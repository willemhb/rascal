/* Core type information, globals, and toplevel expression APIs. */
// headers --------------------------------------------------------------------
#include <string.h>

#include "data/base.h"
#include "data/sym.h"

#include "sys/error.h"
#include "sys/memory.h"

#include "util/hashing.h"
#include "util/number.h"

// macros ---------------------------------------------------------------------
// magic numbers
#define MIN_CAP 8
#define LOADF   0.625

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

#define check_grow(t) ((t)->count >= ((t)->max_count * LOADF))

#define TABLE_IMPL(T, K, V, t, NK, NV, hashf, rehashf, cmpf)            \
  void   init_##t(T* t) {                                               \
    t->kvs       = NULL;                                                \
    t->count     = 0;                                                   \
    t->max_count = 0;                                                   \
  }                                                                     \
                                                                        \
  void free_##t(T* t) {                                                 \
    release(t->kvs, 0);                                                 \
    init_##t(t);                                                        \
  }                                                                     \
                                                                        \
  void init_##t##_kvs(T##KV* kvs, int max_count) {                      \
    for ( int i=0; i < max_count; i++ ) {                               \
      kvs[i].key = NK;                                                  \
      kvs[i].val = NV;                                                  \
    }                                                                   \
  }                                                                     \
                                                                        \
  T##KV* t##_find(T* t, K k, hash_t h) {                                \
    assert(t->kvs != NULL);                                             \
    T##KV* kvs = t->kvs, * kv, * ts = NULL;                             \
    int msk = t->max_count-1;                                           \
    int idx = h & msk;                                                  \
                                                                        \
    for (;;) {                                                          \
      kv = &kvs[idx];                                                   \
      if ( kv->key == NK ) {                                            \
        if ( kv->val != NV ) /* tombstone */                            \
          ts = ts ? : kv;                                               \
        else                                                            \
          break;                                                        \
      } else if ( cmpf(kv->key, k) )                                    \
        break;                                                          \
      idx = (idx + 1) & msk;                                            \
      }                                                                 \
                                                                        \
    return ts ? : kv;                                                   \
    }                                                                   \
                                                                        \
  int rehash_##t( T##KV* old, int omc, T##KV* new, int nmc ) {          \
    int cnt = 0;                                                        \
                                                                        \
    for ( int i=0; i < omc; i++ ) {                                     \
      T##KV* kv = &old[i];                                              \
      if ( kv->key == NK )                                              \
        continue;                                                       \
                                                                        \
      cnt++;                                                            \
                                                                        \
      hash_t hash = rehashf(kv);                                        \
      int msk  = nmc - 1;                                               \
      int idx  = hash & msk;                                            \
      while ( new[idx].key != NK )                                      \
        idx = (idx+1) & msk;                                            \
                                                                        \
      new[idx] = *kv;                                                   \
    }                                                                   \
    return cnt;                                                         \
  }                                                                     \
                                                                        \
    void grow_##t(T* t) {                                               \
      int nmc    = t->max_count < MIN_CAP ? MIN_CAP : t->max_count << 1; \
      T##KV*    nkv = allocate(false, nmc*sizeof(T##KV));               \
                                                                        \
      init_##t##_kvs(nkv, nmc);                                         \
                                                                        \
    if ( t->kvs != NULL ) {                                             \
      int omc = t->max_count;                                           \
      T##KV*    okv = t->kvs;                                           \
      int nc  = rehash_##t(okv, omc, nkv, nmc);                         \
      t->count   = nc;                                                  \
      release(okv, 0);                                                  \
    }                                                                   \
                                                                        \
    t->kvs       = nkv;                                                 \
    t->max_count = nmc;                                                 \
  }                                                                     \
                                                                        \
    bool t##_get(T* t, K k, V* v) {                                     \
    bool out;                                                           \
                                                                        \
    if ( t->kvs == NULL )                                               \
      out = false;                                                      \
    else {                                                              \
      T##KV* kv = t##_find(t, k, hashf(k));                             \
      out       = kv->key != NK;                                        \
                                                                        \
      if ( out && v )                                                   \
        *v = kv->val;                                                   \
    }                                                                   \
                                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool t##_set(T* t, K k, V v) {                                        \
    if ( check_grow(t) )                                                \
      grow_##t(t);                                                      \
                                                                        \
    T##KV* kv = t##_find(t, k, hashf(k));                               \
    bool out  = kv->key == NK;                                          \
                                                                        \
    if ( out )                                                          \
      kv->key = k;                                                      \
    if ( kv->val == NV )                                                \
      t->count++;                                                       \
    kv->val = v;                                                        \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool t##_del( T* t, K k, V* v ) {                                     \
    bool out;                                                           \
                                                                        \
    if ( t->kvs == NULL )                                               \
      out = false;                                                      \
    else {                                                              \
      T##KV* kv = t##_find(t, k, hashf(k));                             \
      out    = kv->key != NK;                                           \
                                                                        \
      if ( out ) {                                                      \
        kv->key = NK;                                                   \
        if ( v )                                                        \
          *v = kv->val;                                                 \
      }                                                                 \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
                                                                        \
  V t##_intern(T* t, K k, T##InternFn ifn) {                            \
    if ( check_grow(t) )                                                \
      grow_##t(t);                                                      \
    hash_t h = hashf(k);                                                \
    T##KV* kv = t##_find(t, k, h);                                      \
                                                                        \
    if ( kv->key == NK ) {                                              \
      if ( kv->val == NV )                                              \
        t->count++;                                                     \
      ifn(t, kv, (void*)k, h);                                          \
    }                                                                   \
                                                                        \
    return kv->val;                                                     \
  }

bool cmp_strings( char * sx, char* sy ) {
  return strcmp(sx, sy) == 0;
}

hash_t rehash_string(StringsKV* kv) {
  Str* s = kv->val;
  return s->hash;
}

TABLE_IMPL(Strings, char*, Str*, strings, NULL, NULL, hash_string, rehash_string, cmp_strings);

bool cmp_symbols( Sym* sx, Sym* sy ) {
  return sx->val == sy->val;
}

hash_t hash_symbol( Sym* sx ) {
  return sx->hash;
}

hash_t rehash_symbol( EnvMapKV* kv ) {
  Sym* s = kv->key;

  return s->hash;
}

TABLE_IMPL(EnvMap, Sym*, Ref*, env_map, NULL, NULL, hash_symbol, rehash_symbol, cmp_symbols );

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------
TypeInfo Types[NUM_TYPES];

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
// Expression APIs ------------------------------------------------------------
ExpType get_exp_type(Expr x) {
  ExpType t;

  switch ( x & XTMSK ) {
    case NUL_T   : t = EXP_NUL;       break;
    case BOOL_T  : t = EXP_BOOL;      break;
    case GLYPH_T : t = EXP_GLYPH;     break;
    case OBJ_T   : t = head(x)->type; break;
    default      : t = EXP_NUM;       break;
  }

  return t;
}

ExpType get_obj_type(void* p) {
  assert(p != NULL);

  return ((Obj*)p)->type;
}

ExpAPI* exp_api(Expr x) {
  TypeInfo* ti = type_info(x);

  return ti->exp_api;
}

ObjAPI* obj_api(void* ptr) {
  assert(ptr != NULL);

  Obj* obj = ptr;

  return Types[obj->type].obj_api;
}

void mark_exp(Expr x) {
  if ( exp_tag(x) == OBJ_T )
    mark_obj(as_obj(x));
}

// Object APIs ----------------------------------------------------------------
void trace_exprs(Exprs* xs) {
  trace_exp_array(xs->count, xs->vals);
}

void trace_objs(Objs* os) {
  trace_obj_array(os->count, os->vals);
}

// object API
void* as_obj(Expr x) {
  return (void*)(x & XVMSK);
}

Expr tag_obj(void* o) {
  return ((Expr)o) | OBJ_T;
}

void* mk_obj(ExpType type, flags_t flags, size_t n) {
  assert(type > EXP_GLYPH);

  ObjAPI* api = Types[type].obj_api;
  
  Obj* out;

  if ( api->alloc_fn )
    out = api->alloc_fn(type, flags, n);

  else
    out = allocate(true, api->obsize);

  out->type     = type;
  out->bfields  = flags | FL_GRAY;
  out->heap     = Heap;
  Heap          = out;

  return out;
}

size_t obj_size(void* ptr) {
  assert(ptr != NULL);

  Obj* obj    = ptr;
  ObjAPI* api = obj_api(obj);
  size_t size = api->size_fn ? api->size_fn(obj) : api->obsize;

  return size;
}

void* clone_obj(void* ptr) {
  assert(ptr != NULL);

  Obj* obj    = ptr;
  ObjAPI* api = obj_api(obj);
  Obj* out    = duplicate(true, obj_size(obj), obj);

  if ( api->clone_fn )
    api->clone_fn(out);

  return out;
}

void mark_obj(void* ptr) {
  Obj* obj = ptr;

  if ( obj != NULL ) {
    if ( obj->black == false ) {
      obj->black = true;

      ObjAPI* api = obj_api(obj);

      if ( api->trace_fn )
        gc_save(obj);

      else
        obj->gray = false;
    }
  }
}

// used mostly to manually unmark global objects so they're collected correctly
// on subsequent GC cycles
void unmark_obj(void* ptr) {
  Obj* obj   = ptr;
  obj->black = false;
  obj->gray  = true;
}

void free_obj(void* ptr) {
  Obj* obj = ptr;

  if ( obj ) {
    ObjAPI* api = obj_api(obj);

    if ( api->free_fn )
      api->free_fn(obj);

    release(obj, obj_size(obj));
  }
}

// HAMT helpers ---------------------------------------------------------------
size_t hamt_shift(void* ob) {
  return ((Obj*)ob)->flags & HAMT_MASK;
}

size_t hamt_asize(void* ob) {
  return ((Obj*)ob)->flags >> HAMT_SHIFT;
}

void init_hamt(void* ob, size_t shift, size_t asize) {
  Obj* obj    = ob;
  obj->flags |= shift | (asize << HAMT_SHIFT);
}

// initialization -------------------------------------------------------------
