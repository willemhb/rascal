#include <string.h>

#include "util/collection.h"
#include "vm.h"
#include "util/util.h"
#include "val.h"

// magic numbers
#define MIN_CAP 8
#define LOADF   0.625

// Exprs implementation -------------------------------------------------------
void init_exprs(RlState* rls, Exprs* a) {
  (void)rls;
  a->data = NULL;
  a->count = 0;
  a->maxc = 0;
}

void free_exprs(RlState* rls, Exprs* a) {
  release(NULL, a->data, 0);
  init_exprs(rls, a);
}

void grow_exprs(RlState* rls, Exprs* a) {
  assert(a->maxc < MAX_ARITY);
  (void)rls;
  int new_maxc = a->maxc ? a->maxc << 1 : MIN_CAP;
  Expr* new_spc = reallocate(NULL,
                             new_maxc * sizeof(Expr),
                             a->maxc * sizeof(Expr),
                             a->data);

  a->data = new_spc;
  a->maxc = new_maxc;
}

void shrink_exprs(RlState* rls, Exprs* a) {
  assert(a->maxc > MIN_CAP);
  (void)rls;

  size_t new_maxc = a->maxc >> 1;
  Expr* new_spc = reallocate(NULL,
                             new_maxc * sizeof(Expr),
                             a->maxc * sizeof(Expr),
                             a->data);

  a->data = new_spc;
  a->maxc = new_maxc;
}

void resize_exprs(RlState* rls, Exprs* a, int n) {
  (void)rls;
  int new_maxc = cpow2(n);

  if ( new_maxc < MIN_CAP )
    new_maxc = MIN_CAP;

  Expr* new_spc = reallocate(NULL,
                             new_maxc * sizeof(Expr),
                             a->maxc * sizeof(Expr),
                             a->data);
  a->data = new_spc;
  a->maxc = new_maxc;
}

void exprs_push(RlState* rls, Exprs* a, Expr x) {
  if ( a->count == a->maxc )
    grow_exprs(rls, a);

  a->data[a->count++] = x;
}

Expr exprs_pop(RlState* rls, Exprs* a) {
  Expr out = a->data[--a->count];

  if ( a->count == 0 )
    free_exprs(rls, a);
  else if ( a->maxc > MIN_CAP && a->count < (a->maxc >> 1) )
    shrink_exprs(rls, a);

  return out;
}

void exprs_write(RlState* rls, Exprs* a, Expr* xs, int n) {
  if ( a->count + n > a->maxc )
    resize_exprs(rls, a, a->count + n);

  if ( xs != NULL )
    memcpy(a->data + a->count, xs, n * sizeof(Expr));

  a->count += n;
}

// Objs implementation --------------------------------------------------------
void init_objs(RlState* rls, Objs* a) {
  (void)rls;
  a->data      = NULL;
  a->count     = 0;
  a->maxc = 0;
}

void free_objs(RlState* rls, Objs* a) {
  release(NULL, a->data, 0);
  init_objs(rls, a);
}

void grow_objs(RlState* rls, Objs* a) {
  (void)rls;
  int new_maxc = a->maxc ? a->maxc << 1 : MIN_CAP;
  void** new_spc = reallocate(NULL,
                              new_maxc * sizeof(void*),
                              a->maxc * sizeof(void*),
                              a->data);

  a->data = new_spc;
  a->maxc = new_maxc;
}

void shrink_objs(RlState* rls, Objs* a) {
  assert(a->maxc > MIN_CAP);
  (void)rls;

  size_t new_maxc = a->maxc >> 1;
  void** new_spc = reallocate(NULL,
                              new_maxc * sizeof(void*),
                              a->maxc * sizeof(void*),
                              a->data);

  a->data = new_spc;
  a->maxc = new_maxc;
}

void resize_objs(RlState* rls, Objs* a, int n) {
  (void)rls;
  int new_maxc = cpow2(n);

  if ( new_maxc < MIN_CAP )
    new_maxc = MIN_CAP;

  void** new_spc = reallocate(NULL,
                              new_maxc * sizeof(void*),
                              a->maxc * sizeof(void*),
                              a->data);
  a->data = new_spc;
  a->maxc = new_maxc;
}

void objs_push(RlState* rls, Objs* a, void* x) {
  if ( a->count == a->maxc )
    grow_objs(rls, a);

  a->data[a->count++] = x;
}

void* objs_pop(RlState* rls, Objs* a) {
  void* out = a->data[--a->count];

  if ( a->count == 0 )
    free_objs(rls, a);
  else if ( a->maxc > MIN_CAP && a->count < (a->maxc >> 1) )
    shrink_objs(rls, a);

  return out;
}

void objs_write(RlState* rls, Objs* a, void** xs, int n) {
  if ( a->count + n > a->maxc )
    resize_objs(rls, a, a->count + n);

  if ( xs != NULL )
    memcpy(a->data + a->count, xs, n * sizeof(void*));

  a->count += n;
}

// Text_Buf implementation -------------------------------------------------------
void init_text_buf(RlState* rls, TextBuf* a) {
  (void)rls;
  a->data     = NULL;
  a->count     = 0;
  a->maxc = 0;
}

void free_text_buf(RlState* rls, TextBuf* a) {
  release(NULL, a->data, 0);
  init_text_buf(rls, a);
}

void grow_text_buf(RlState* rls, TextBuf* a) {
  (void)rls;
  int new_maxc = a->maxc ? a->maxc << 1 : MIN_CAP;
  char* new_spc = reallocate(NULL,
                                 new_maxc * sizeof(char),
                                 a->maxc * sizeof(char),
                                 a->data);

  a->data = new_spc;
  a->maxc = new_maxc;
}

void shrink_text_buf(RlState* rls, TextBuf* a) {
  assert(a->maxc > MIN_CAP);
  (void)rls;

  size_t new_maxc = a->maxc >> 1;
  char* new_spc = reallocate(NULL,
                                 new_maxc * sizeof(char),
                                 a->maxc * sizeof(char),
                                 a->data);

  a->data = new_spc;
  a->maxc = new_maxc;
}

void resize_text_buf(RlState* rls, TextBuf* a, int n) {
  (void)rls;
  int new_maxc = cpow2(n+1);

  if ( new_maxc < MIN_CAP )
    new_maxc = MIN_CAP;

  char* new_spc = reallocate(NULL,
                                 new_maxc * sizeof(char),
                                 a->maxc * sizeof(char),
                                 a->data);
  a->data = new_spc;
  a->maxc = new_maxc;
}

void text_buf_push(RlState* rls, TextBuf* a, char x) {
  if ( a->count + 1 >= a->maxc )
    grow_text_buf(rls, a);

  a->data[a->count++] = x;
}

char text_buf_pop(RlState* rls, TextBuf* a) {
  char out = a->data[--a->count];

  if ( a->count == 0 )
    free_text_buf(rls, a);
  else if ( a->maxc > MIN_CAP && a->count-1 < (a->maxc >> 1) )
    shrink_text_buf(rls, a);

  return out;
}

void text_buf_write(RlState* rls, TextBuf* a, char* xs, int n) {
  if ( a->count + n + 1 > a->maxc )
    resize_text_buf(rls, a, a->count + n);

  if ( xs != NULL )
    memcpy(a->data + a->count, xs, n * sizeof(char));

  a->count += n;
}

// Code_Buf implementation -------------------------------------------------------
void init_code_buf(RlState* rls, CodeBuf* a) {
  (void)rls;
  a->data = NULL;
  a->count = 0;
  a->maxc = 0;
}

void free_code_buf(RlState* rls, CodeBuf* a) {
  release(NULL, a->data, 0);
  init_code_buf(rls, a);
}

void grow_code_buf(RlState* rls, CodeBuf* a) {
  (void)rls;
  int new_maxc = a->maxc ? a->maxc << 1 : MIN_CAP;
  instr_t* new_spc = reallocate(NULL,
                                 new_maxc * sizeof(instr_t),
                                 a->maxc * sizeof(instr_t),
                                 a->data);

  a->data = new_spc;
  a->maxc = new_maxc;
}

void shrink_code_buf(RlState* rls, CodeBuf* a) {
  assert(a->maxc > MIN_CAP);
  (void)rls;

  size_t new_maxc = a->maxc >> 1;
  instr_t* new_spc = reallocate(NULL,
                                 new_maxc * sizeof(instr_t),
                                 a->maxc * sizeof(instr_t),
                                 a->data);

  a->data = new_spc;
  a->maxc = new_maxc;
}

void resize_code_buf(RlState* rls, CodeBuf* a, int n) {
  (void)rls;
  int new_maxc = cpow2(n);

  if ( new_maxc < MIN_CAP )
    new_maxc = MIN_CAP;

  instr_t* new_spc = reallocate(NULL,
                                 new_maxc * sizeof(instr_t),
                                 a->maxc * sizeof(instr_t),
                                 a->data);
  a->data = new_spc;
  a->maxc = new_maxc;
}

void code_buf_push(RlState* rls, CodeBuf* a, instr_t x) {
  if ( a->count == a->maxc )
    grow_code_buf(rls, a);

  a->data[a->count++] = x;
}

instr_t code_buf_pop(RlState* rls, CodeBuf* a) {
  instr_t out = a->data[--a->count];

  if ( a->count == 0 )
    free_code_buf(rls, a);
  else if ( a->maxc > MIN_CAP && a->count < (a->maxc >> 1) )
    shrink_code_buf(rls, a);

  return out;
}

void code_buf_write(RlState* rls, CodeBuf* a, instr_t* xs, int n) {
  if ( a->count + n > a->maxc )
    resize_code_buf(rls, a, a->count + n);

  if ( xs != NULL )
    memcpy(a->data + a->count, xs, n * sizeof(instr_t));

  a->count += n;
}

void init_line_info(RlState* rls, LineInfo* a) {
  (void)rls;
  a->data = NULL;
  a->count = 0;
  a->maxc = 0;
}

void free_line_info(RlState* rls, LineInfo* a) {
  release(NULL, a->data, 0);
  init_line_info(rls, a);
}

void grow_line_info(RlState* rls, LineInfo* a) {
  (void)rls;
  int new_maxc = a->maxc ? a->maxc << 1 : MIN_CAP;
  int* new_spc = reallocate(NULL,
                            new_maxc * sizeof(int),
                            a->maxc * sizeof(int),
                            a->data);

  a->data = new_spc;
  a->maxc = new_maxc;
}

void add_to_line_info(RlState* rls, LineInfo* a, int line, int max_offset) {
  if ( a->count + 2 >= a->maxc )
    grow_line_info(rls, a);

  a->data[a->count++] = line;
  a->data[a->count++] = max_offset;
}

// BitVec implementation ------------------------------------------------------
#define BITMAP_MASK 0xfffffffffffffffful

static bool bitmap_has(uintptr_t map, int n) {
  if ( n < 0 || n > MAX_FARGC )
    return false;

  return (1ul << n) & map;
}

static int bitmap_to_index(uintptr_t map, int n) {
  if ( n == 0 )
    return 0;

  return popc(((1ul << n) -1) & map);
}


static void bitmap_set(uintptr_t* map, int n) {
  assert(n >= 0 && n < MAX_FARGC);
  *map |= 1ul << n;
}

void init_bit_vec(RlState* rls, BitVec* bv) {
  (void)rls;
  bv->data = NULL;
  bv->count = 0;
  bv->maxc = 0;
  bv->bitmap = 0;
}

void free_bit_vec(RlState* rls, BitVec* bv) {
  release(NULL, bv->data, 0);
  init_bit_vec(rls, bv);
}

void grow_bit_vec(RlState* rls, BitVec* bv) {
  (void)rls;

  int current_size = bv->maxc;
  int new_size = current_size == 0 ? MIN_CAP : bv->maxc << 1;
  assert(new_size < MAX_FARGC);

  if ( current_size == 0 )
    bv->data = allocate(NULL,
                        new_size*sizeof(void*));

  else
    bv->data = reallocate(NULL,
                          new_size*sizeof(void*),
                          current_size*sizeof(void*),
                          bv->data);

  bv->maxc = new_size;
}

bool bit_vec_has(BitVec* bv, int n) {
  return bitmap_has(bv->bitmap, n);
}

void bit_vec_set(RlState* rls, BitVec* bv, int n, void* d) {
  assert(!bit_vec_has(bv, n));

  if ( bv->count + 1 > bv->maxc )
    grow_bit_vec(rls, bv);

  int i = bitmap_to_index(bv->bitmap, n);

  if ( bv->count > 0 && i <= bv->count )
    memmove(bv->data+i+1, bv->data+i, (bv->count-i)*sizeof(void*));

  bv->data[i] = d;
  bitmap_set(&bv->bitmap, n);
  bv->count++;
}

void* bit_vec_get(BitVec* bv, int n) {
  if ( !bit_vec_has(bv, n) )
    return NULL;

  int i = bitmap_to_index(bv->bitmap, n);
  return bv->data[i];
}

// Strings table implementation -----------------------------------------------
#define check_grow(t) ((t)->count >= ((t)->maxc * LOADF))

static void init_strings_kvs(StringsKV* kvs, int maxc) {
  for ( int i = 0; i < maxc; i++ ) {
    kvs[i].key = NULL;
    kvs[i].val = NULL;
  }
}

static bool cmp_strings(char* sx, char* sy) {
  return strcmp(sx, sy) == 0;
}

static hash_t rehash_string(StringsKV* kv) {
  Str* s = kv->val;
  return s->hash;
}

static StringsKV* strings_find(Strings* t, char* k, hash_t h) {
  assert(t->kvs != NULL);
  StringsKV* kvs = t->kvs;
  StringsKV* kv;
  StringsKV* ts = NULL;
  int msk = t->maxc - 1;
  int idx = h & msk;

  for (;;) {
    kv = &kvs[idx];
    if ( kv->key == NULL ) {
      if ( kv->val != NULL ) // tombstone
        ts = ts ? ts : kv;
      else
        break;
    } else if ( cmp_strings(kv->key, k) )
      break;
    idx = (idx + 1) & msk;
  }

  return ts ? ts : kv;
}

static int rehash_strings(StringsKV* old, int omc, StringsKV* new, int nmc) {
  int cnt = 0;

  for ( int i = 0; i < omc; i++ ) {
    StringsKV* kv = &old[i];
    if ( kv->key == NULL )
      continue;

    cnt++;

    hash_t hash = rehash_string(kv);
    int msk = nmc - 1;
    int idx = hash & msk;
    while ( new[idx].key != NULL )
      idx = (idx + 1) & msk;

    new[idx] = *kv;
  }
  return cnt;
}

static void grow_strings(RlState* rls, Strings* t) {
  (void)rls;

  int nmc = t->maxc < MIN_CAP ? MIN_CAP : t->maxc << 1;
  StringsKV* nkv = allocate(NULL, nmc * sizeof(StringsKV));

  init_strings_kvs(nkv, nmc);

  if ( t->kvs != NULL ) {
    int omc = t->maxc;
    StringsKV* okv = t->kvs;
    int nc = rehash_strings(okv, omc, nkv, nmc);
    t->count = nc;
    release(NULL, okv, 0);
  }

  t->kvs = nkv;
  t->maxc = nmc;
}

void init_strings(RlState* rls, Strings* t) {
  (void)rls;
  t->kvs       = NULL;
  t->count     = 0;
  t->maxc = 0;
}

void free_strings(RlState* rls, Strings* t) {
  release(NULL, t->kvs, 0);
  init_strings(rls, t);
}

bool strings_get(RlState* rls, Strings* t, char* k, Str** v) {
  (void)rls;
  bool out;

  if ( t->kvs == NULL )
    out = false;
  else {
    StringsKV* kv = strings_find(t, k, hash_string(k));
    out = kv->key != NULL;

    if ( out && v )
      *v = kv->val;
  }

  return out;
}

bool strings_set(RlState* rls, Strings* t, char* k, Str* v) {
  if ( check_grow(t) )
    grow_strings(rls, t);

  StringsKV* kv = strings_find(t, k, hash_string(k));
  bool out = kv->key == NULL;

  if ( out )
    kv->key = k;
  if ( kv->val == NULL )
    t->count++;
  kv->val = v;
  return out;
}

bool strings_del(RlState* rls, Strings* t, char* k, Str** v) {
  (void)rls;
  bool out;

  if ( t->kvs == NULL )
    out = false;
  else {
    StringsKV* kv = strings_find(t, k, hash_string(k));
    out = kv->key != NULL;

    if ( out ) {
      kv->key = NULL;
      if ( v )
        *v = kv->val;
    }
  }
  return out;
}

Str* strings_intern(RlState* rls, Strings* t, char* k, StringsInternFn ifn) {
  if ( check_grow(t) )
    grow_strings(rls, t);

  hash_t h = hash_string(k);
  StringsKV* kv = strings_find(t, k, h);

  if ( kv->key == NULL ) {
    if ( kv->val == NULL )
      t->count++;
    ifn(rls, t, kv, k, h);
  }

  return kv->val;
}

// EMap table implementation --------------------------------------------------
static void init_emap_kvs(EMapKV* kvs, int maxc) {
  for ( int i = 0; i < maxc; i++ ) {
    kvs[i].key = NULL;
    kvs[i].val = NULL;
  }
}

static bool cmp_symbols(Sym* sx, Sym* sy) {
  return sx->val == sy->val;
}

static hash_t hash_symbol(Sym* sx) {
  return sx->hash;
}

static hash_t rehash_symbol(EMapKV* kv) {
  Sym* s = kv->key;
  return s->hash;
}

static EMapKV* emap_find(EMap* t, Sym* k, hash_t h) {
  assert(t->kvs != NULL);
  EMapKV* kvs = t->kvs;
  EMapKV* kv;
  EMapKV* ts = NULL;
  int msk = t->maxc - 1;
  int idx = h & msk;

  for (;;) {
    kv = &kvs[idx];
    if ( kv->key == NULL ) {
      if ( kv->val != NULL ) // tombstone
        ts = ts ? ts : kv;
      else
        break;
    } else if ( cmp_symbols(kv->key, k) )
      break;
    idx = (idx + 1) & msk;
  }

  return ts ? ts : kv;
}

static int rehash_emap(EMapKV* old, int omc, EMapKV* new, int nmc) {
  int cnt = 0;

  for ( int i = 0; i < omc; i++ ) {
    EMapKV* kv = &old[i];
    if ( kv->key == NULL )
      continue;

    cnt++;

    hash_t hash = rehash_symbol(kv);
    int msk = nmc - 1;
    int idx = hash & msk;
    while ( new[idx].key != NULL )
      idx = (idx + 1) & msk;

    new[idx] = *kv;
  }
  return cnt;
}

static void grow_emap(RlState* rls, EMap* t) {
  (void)rls;
  int nmc = t->maxc < MIN_CAP ? MIN_CAP : t->maxc << 1;
  EMapKV* nkv = allocate(NULL, nmc * sizeof(EMapKV));

  init_emap_kvs(nkv, nmc);

  if ( t->kvs != NULL ) {
    int omc = t->maxc;
    EMapKV* okv = t->kvs;
    int nc = rehash_emap(okv, omc, nkv, nmc);
    t->count = nc;
    release(NULL, okv, 0);
  }

  t->kvs = nkv;
  t->maxc = nmc;
}

void init_emap(RlState* rls, EMap* t) {
  (void)rls;
  t->kvs   = NULL;
  t->count = 0;
  t->maxc  = 0;
}

void free_emap(RlState* rls, EMap* t) {
  release(NULL, t->kvs, 0);
  init_emap(rls, t);
}

bool emap_get(RlState* rls, EMap* t, Sym* k, Ref** v) {
  (void)rls;
  bool out;

  if ( t->kvs == NULL )
    out = false;
  else {
    EMapKV* kv = emap_find(t, k, hash_symbol(k));
    out = kv->key != NULL;

    if ( out && v )
      *v = kv->val;
  }

  return out;
}

bool emap_set(RlState* rls, EMap* t, Sym* k, Ref* v) {
  if ( check_grow(t) )
    grow_emap(rls, t);

  EMapKV* kv = emap_find(t, k, hash_symbol(k));
  bool out = kv->key == NULL;

  if ( out )
    kv->key = k;
  if ( kv->val == NULL )
    t->count++;
  kv->val = v;
  return out;
}

bool emap_del(RlState* rls, EMap* t, Sym* k, Ref** v) {
  (void)rls;
  bool out;

  if ( t->kvs == NULL )
    out = false;
  else {
    EMapKV* kv = emap_find(t, k, hash_symbol(k));
    out = kv->key != NULL;

    if ( out ) {
      kv->key = NULL;
      if ( v )
        *v = kv->val;
    }
  }
  return out;
}

Ref* emap_intern(RlState* rls, EMap* t, Sym* k, EMapInternFn ifn) {
  if ( check_grow(t) )
    grow_emap(rls, t);

  hash_t h = hash_symbol(k);
  EMapKV* kv = emap_find(t, k, h);

  if ( kv->key == NULL ) {
    if ( kv->val == NULL )
      t->count++;
    ifn(rls, t, kv, k, h);
  }

  return kv->val;
}
