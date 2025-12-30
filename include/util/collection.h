#ifndef rl_util_collection_h
#define rl_util_collection_h

#include "common.h"

// Array types ----------------------------------------------------------------
// Exprs - array of Expr values
struct Exprs {
  Expr* data;
  int count, maxc;
};

void init_exprs(RlState* rls, Exprs* a);
void free_exprs(RlState* rls, Exprs* a);
void grow_exprs(RlState* rls, Exprs* a);
void shrink_exprs(RlState* rls, Exprs* a);
void resize_exprs(RlState* rls, Exprs* a, int n);
void exprs_push(RlState* rls, Exprs* a, Expr x);
Expr exprs_pop(RlState* rls, Exprs* a);
void exprs_write(RlState* rls, Exprs* a, Expr* xs, int n);

// Objs - array of void* values
struct Objs {
  void** data;
  int count, maxc;
};

void init_objs(RlState* rls, Objs* a);
void free_objs(RlState* rls, Objs* a);
void grow_objs(RlState* rls, Objs* a);
void shrink_objs(RlState* rls, Objs* a);
void resize_objs(RlState* rls, Objs* a, int n);
void objs_push(RlState* rls, Objs* a, void* x);
void* objs_pop(RlState* rls, Objs* a);
void objs_write(RlState* rls, Objs* a, void** xs, int n);

// generic buffer type for binary or text data
/* typedef struct Buffer { */
/*   void* data; */
/*   int count, maxc, elsize; */
/*   bool encoded; */
/* } Buffer; */

/* void init_buffer(RlState* rls, Buffer* a); */
/* void free_buffer(RlState* rls, Buffer* a); */
/* void grow_buffer(RlState* rls, Buffer* a); */
/* void shrink_buffer(RlState* rls, Buffer* a); */
/* void resize_buffer(RlState* rls, Buffer* a, int n); */
/* void buffer_push(RlState* rls, Buffer* a, int x); */
/* int buffer_pop(RlState* rls, Buffer* a); */
/* void buffer_write(RlState* rls, Buffer* a, void* xs, int n); */

// CodeBuf - array of instr_t values
struct CodeBuf {
  instr_t* data;
  int count, maxc;
};

void init_code_buf(RlState* rls, CodeBuf* a);
void free_code_buf(RlState* rls, CodeBuf* a);
void grow_code_buf(RlState* rls, CodeBuf* a);
void shrink_code_buf(RlState* rls, CodeBuf* a);
void resize_code_buf(RlState* rls, CodeBuf* a, int n);
void code_buf_push(RlState* rls, CodeBuf* a, ushort_t x);
instr_t code_buf_pop(RlState* rls, CodeBuf* a);
void code_buf_write(RlState* rls, CodeBuf* a, instr_t* xs, int n);

struct TextBuf {
  char* data;
  int count, maxc;
};

void init_text_buf(RlState* rls, TextBuf* a);
void free_text_buf(RlState* rls, TextBuf* a);
void grow_text_buf(RlState* rls, TextBuf* a);
void shrink_text_buf(RlState* rls, TextBuf* a);
void resize_text_buf(RlState* rls, TextBuf* a, int n);
void text_buf_push(RlState* rls, TextBuf* a, char x);
char text_buf_pop(RlState* rls, TextBuf* a);
void text_buf_write(RlState* rls, TextBuf* a, char* xs, int n);

struct LineInfo {
  int* data;
  int count, maxc;
};

void init_line_info(RlState* rls, LineInfo* li);
void free_line_info(RlState* rls, LineInfo* li);
void grow_line_info(RlState* rls, LineInfo* li);
void add_to_line_info(RlState* rls, LineInfo* li, int line, int max_offset);

// bitmapped vector, used to implement methoed tables and eventually HAMTs
struct BitVec {
  void** data;
  int count, maxc;
  uintptr_t bitmap;
};

int bitmap_to_index(uintptr_t map, int n);
void init_bit_vec(RlState* rls, BitVec* bv);
void free_bit_vec(RlState* rls, BitVec* bv);
void grow_bit_vec(RlState* rls, BitVec* bv);
bool bit_vec_has(BitVec* bv, int n);
void bit_vec_set(RlState* rls, BitVec* bv, int n, void* d);
void* bit_vec_get(BitVec* bv, int n);
void* bit_vec_update(RlState* rls, BitVec* bv, int n, void* d);
void bit_vec_clone(RlState* rls, BitVec* src, BitVec* dst);
void bit_vec_remove(RlState* rls, BitVec* bv, int n);

// Table types ----------------------------------------------------------------
// generic table key/value pair
typedef struct {
  void* key;
  void* val;
} KV;

// table operations
typedef hash_t (*TableHashFn)(void* k);
typedef hash_t (*TableRehashFn)(KV* kv);
typedef bool   (*TableCmpFn)(void* x, void* y);
typedef void   (*TableInternFn)(RlState* rls, Table* table, KV* kv, void* key, hash_t hash);
typedef void   (*TableWriteFn)(void* val, void* spc);
typedef void   (*TableMarkFn)(RlState* rls, Table* table, KV* kv);

// fully generic table type
struct Table {
  KV* kvs;
  int count, maxc;
  bool weak_key, weak_val;

  // the value to mark unused entries
  void* sentinel;

  TableHashFn hash;
  TableRehashFn rehash;
  TableCmpFn compare;
  TableInternFn intern;
  TableWriteFn init;
  TableMarkFn mark;
};

// Prototypes -----------------------------------------------------------------
// table type
void   init_table(RlState* rls, Table* table);
void   free_table(RlState* rls, Table* table);
bool   table_get(RlState* rls, Table* table, void* key, void* result);
bool   table_has(RlState* rls, Table* table, void* key);
bool   table_set(RlState* rls, Table* table, void* key, void* val, void* result);
bool   table_del(RlState* rls, Table* table, void* key, void* result);
bool   table_intern(RlState* rls, Table* table, void* key, void* result);

// Strings - hash table mapping char* to Str*
typedef struct StringsKV {
  char* key;
  Str*  val;
} StringsKV;

typedef struct Strings {
  StringsKV* kvs;
  int count, maxc;
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
  int count, maxc;
} EMap;

typedef void (*EMapInternFn)(RlState* rls, EMap* t, EMapKV* kv, Sym* k, hash_t h);

void init_emap(RlState* rls, EMap* t);
void free_emap(RlState* rls, EMap* t);
bool emap_get(RlState* rls, EMap* t, Sym* k, Ref** v);
bool emap_set(RlState* rls, EMap* t, Sym* k, Ref* v);
bool emap_del(RlState* rls, EMap* t, Sym* k, Ref** v);
Ref* emap_intern(RlState* rls, EMap* t, Sym* k, EMapInternFn ifn);

#endif
