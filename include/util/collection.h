#ifndef rl_util_collection_h
#define rl_util_collection_h

#include "common.h"

// Array types ----------------------------------------------------------------
// Exprs - array of Expr values
typedef struct Exprs {
  Expr* vals;
  int count, max_count;
} Exprs;

void init_exprs(RlState* rls, Exprs* a);
void free_exprs(RlState* rls, Exprs* a);
void grow_exprs(RlState* rls, Exprs* a);
void shrink_exprs(RlState* rls, Exprs* a);
void resize_exprs(RlState* rls, Exprs* a, int n);
void exprs_push(RlState* rls, Exprs* a, Expr x);
Expr exprs_pop(RlState* rls, Exprs* a);
void exprs_write(RlState* rls, Exprs* a, Expr* xs, int n);

// Objs - array of void* values
typedef struct Objs {
  void** vals;
  int count, max_count;
} Objs;

void  init_objs(RlState* rls, Objs* a);
void  free_objs(RlState* rls, Objs* a);
void  grow_objs(RlState* rls, Objs* a);
void  shrink_objs(RlState* rls, Objs* a);
void  resize_objs(RlState* rls, Objs* a, int n);
void  objs_push(RlState* rls, Objs* a, void* x);
void* objs_pop(RlState* rls, Objs* a);
void  objs_write(RlState* rls, Objs* a, void** xs, int n);

// Bin16 - array of ushort_t values
typedef struct Bin16 {
  ushort_t* vals;
  int count, max_count;
} Bin16;

void     init_bin16(RlState* rls, Bin16* a);
void     free_bin16(RlState* rls, Bin16* a);
void     grow_bin16(RlState* rls, Bin16* a);
void     shrink_bin16(RlState* rls, Bin16* a);
void     resize_bin16(RlState* rls, Bin16* a, int n);
void     bin16_push(RlState* rls, Bin16* a, ushort_t x);
ushort_t bin16_pop(RlState* rls, Bin16* a);
void     bin16_write(RlState* rls, Bin16* a, ushort_t* xs, int n);

// Table types ----------------------------------------------------------------
// Strings - hash table mapping char* to Str*
typedef struct StringsKV {
  char* key;
  Str*  val;
} StringsKV;

typedef struct Strings {
  StringsKV* kvs;
  int count, max_count;
} Strings;

typedef void (*StringsInternFn)(RlState* rls, Strings* t, StringsKV* kv, char* k, hash_t h);

void init_strings(RlState* rls, Strings* t);
void free_strings(RlState* rls, Strings* t);
bool strings_get(RlState* rls, Strings* t, char* k, Str** v);
bool strings_set(RlState* rls, Strings* t, char* k, Str* v);
bool strings_del(RlState* rls, Strings* t, char* k, Str** v);
Str* strings_intern(RlState* rls, Strings* t, char* k, StringsInternFn ifn);

// EMap - hash table mapping Sym* to Ref*
typedef struct EMapKV {
  Sym* key;
  Ref* val;
} EMapKV;

typedef struct EMap {
  EMapKV* kvs;
  int count, max_count;
} EMap;

typedef void (*EMapInternFn)(RlState* rls, EMap* t, EMapKV* kv, Sym* k, hash_t h);

void init_emap(RlState* rls, EMap* t);
void free_emap(RlState* rls, EMap* t);
bool emap_get(RlState* rls, EMap* t, Sym* k, Ref** v);
bool emap_set(RlState* rls, EMap* t, Sym* k, Ref* v);
bool emap_del(RlState* rls, EMap* t, Sym* k, Ref** v);
Ref* emap_intern(RlState* rls, EMap* t, Sym* k, EMapInternFn ifn);

#endif
