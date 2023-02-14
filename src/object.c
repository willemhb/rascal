#include <assert.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "hash.h"
#include "compare.h"

/* globals */
usize ObjSize[NUM_OBJ_TYPES] = {
  [SYM_OBJ] = sizeof(Sym), [LIST_OBJ]  = sizeof(List),  [PAIR_OBJ] = sizeof(Pair),
  [VEC_OBJ] = sizeof(Vec), [TABLE_OBJ] = sizeof(Table), [BIN_OBJ]  = sizeof(Bin),
  
  [CLOSURE_OBJ] = sizeof(Closure), [CHUNK_OBJ] = sizeof(Chunk),
  [NATIVE_OBJ]  = sizeof(Native),  [UPVAL_OBJ] = sizeof(UpVal)
};

/* empty singletons */
struct List EmptyList = {
  .obj={
    .next =NULL,
    .type =LIST_OBJ,
    .flags=STATIC_OBJ,
    .black=true,
    .gray =false
  },
  .tail =&EmptyList,
  .arity=0,
  .head=NIL_VAL
};

Sym *SymbolTable = NULL;

/* API */
/* utility */
usize obj_size(Obj *obj) {
  assert(obj);

  return ObjSize[obj->type];
}

bool is_obj_type(Val value, ObjType type) {
  return IS_OBJ(value) && OBJ_TYPE(value) == type;
}

bool is_static_obj(Obj *obj) {
  return obj && !!(obj->flags&STATIC_OBJ);
}

/* internal helpers */
void init_obj(Obj *self, ObjType type, ObjFl fl);

Obj *new_obj(ObjType type, ObjFl fl) {
  Obj *out = allocate(ObjSize[type]);
  init_obj(out, type, fl);
  return out;
}

void init_obj(Obj *self, ObjType type, ObjFl fl) {
  self->next   = Heap.live;
  Heap.live    = self;
  self->type   = type;
  self->flags |= fl;
  self->gray   = true;
  self->black  = false;
}

Sym **find_sym(char *name) {
  Sym **loc = &SymbolTable;

  while (*loc) {
    int o = strcmp(name, (*loc)->name);

    if (o < 0)
      loc = &(*loc)->left;

    else if (o > 0)
      loc = &(*loc)->right;

    else
      break;
  }

  return loc;
}

usize pad_array_size(usize count, usize cap) {
  if (cap < CAP_MIN)
    cap = CAP_MIN;

  while (count > cap)
    cap <<= 1;

  while (count < (cap >> 1) && cap > CAP_MIN)
    cap >>= 1;

  return cap;
}

Val *vec_peep(Vec *vec, int i) {
  if (i < 0)
    i += vec->count;

  assert(i >= 0 && (usize)i < vec->count);

  return vec->array+i;
}

usize pad_table_size(usize count, usize cap) {
  if (cap < CAP_MIN)
    cap = CAP_MIN;

  while (count > (cap * TABLE_PRESSURE))
    cap <<= 1;

  while (count < ((cap >> 1) * TABLE_PRESSURE) && cap > CAP_MIN)
    cap >>= 1;

  return cap;
}

void rehash_table(Table *table, usize new_cap) {
  Pair **new_spc = allocate(new_cap * sizeof(Pair*));

  for (usize i=0; i<table->cap; i++) {
    Pair *entry = table->table[i];

    if (entry == NULL)
      continue;

    uhash h   = entry->hash;
    usize idx = h & (new_cap-1);

    while (new_spc[idx])
      idx = (idx+1) & (new_cap-1);

    new_spc[idx] = entry;
  }
  deallocate(table->table, table->cap * sizeof(Pair*));
  table->table = new_spc;
}

Pair **table_find(Table *table, Val key) {
  uhash h = hash_val(key);
  usize m = table->cap-1;
  usize i = h & m;
  Pair **entry;

  while (*(entry=table->table+i)) {
    if (equal(key, (*entry)->car))
      break;

    i = (i+1) & m;
  }

  return entry;
}

/* constructors */
Sym *new_sym(char *name) {
  static uint64 counter = 1;

  usize namelen = strlen(name);

  assert(namelen > 0);

  uhash base_hash = mix_hashes(hash_uint(SYM_OBJ), hash_uint(counter));
  uhash name_hash = hash_str(name);
  uhash sym_hash  = mix_hashes(base_hash, name_hash);

  Sym *out = (Sym*)new_obj(SYM_OBJ, 0);
  
  out->name       = duplicate(name, namelen+1);
  out->hash       = sym_hash;
  out->idno       = counter++;
  out->bind       = NIL_VAL;
  out->left       = NULL;
  out->right      = NULL;
  
  out->obj.flags |= (*name == ':') * LITERAL_SYM;

  return out;
}

Val sym(char *name) {
  Sym **loc = find_sym(name);

  if (*loc == NULL)
    *loc = new_sym(name);

  Obj *out = *(Obj**)loc;

  return mk_obj(out);
}

List *new_list(int n_args, Val *args) {
  assert(n_args > 0);

  List *out   = allocate(n_args * sizeof(List));
  List *curr  = out;

  for (int i=0, arity=n_args; i<n_args; i++, arity--, curr++) {
    init_obj((Obj*)curr, LIST_OBJ, 0);

    curr->head  = args[i];
    curr->arity = arity;

    if (arity == 1)
      curr->tail = &EmptyList;

    else
      curr->tail = (curr+1);
  }

  return out;
}

Val list(int n_args, Val *args) {
  List *out = n_args ? new_list(n_args, args) : &EmptyList;

  return mk_obj((Obj*)out);
}

Vec *new_vec(int n_args, Val *args) {
  usize count = n_args, cap = pad_array_size(count, 0);

  Vec *out   = (Vec*)new_obj(VEC_OBJ, 0);
  out->count = count;
  out->cap   = cap;
  out->array = allocate(cap * sizeof(Val));

  if (args)
    memcpy(out->array, args, count * sizeof(Val));

  return out;
}

Val vec(int n_args, Val *args) {
  Vec *out = new_vec(n_args, args);

  return mk_obj((Obj*)out);
}

/* accessors & mutators */
/* vec API */
void vec_clear(Vec *vec) {
  pad_vec(vec, 0);
  vec->count = 0;
  memset(vec->array, 0, vec->cap * sizeof(Val));
}

Val vec_ref(Vec *vec, int i) {
  return *vec_peep(vec, i);
}

Val vec_set(Vec *vec, int i, Val x) {
  return (*vec_peep(vec, i) = x);
}

usize vec_push(Vec *vec, Val x) {
  pad_vec(vec, ++vec->count);
  vec_set(vec, -1, x);

  return vec->count;
}

Val vec_pop(Vec *vec) {
  assert(vec->count);
  Val out = vec_ref(vec, -1);
  pad_vec(vec, --vec->count);
  return out;
}

/* table API */
void table_clear(Table *table) {
  pad_table(table, 0);
  table->count = 0;
  memset(table->table, 0, table->cap * sizeof(Pair*));
}

bool table_ref(Table *table, Val key, Val *buf) {
  Pair **loc = table_find(table, key);

  bool out = *loc;

  if (buf)
    *buf = out ? (*loc)->cdr : NIL_VAL;

  return out;
}

bool table_set(Table *table, Val key, Val val) {
  pad_table(table, table->count+1);

  Pair **loc = table_find(table, key);

  bool out = !*loc;

  if (out) {
    *loc = new_pair(key, NIL_VAL);
    table->count++;
  }

  (*loc)->cdr = val;

  return out;
}

bool table_pop(Table *table, Val key, Pair **buf) {
  Pair **loc = table_find(table, key);
  bool out = *loc;

  if (out) {
    if (*buf)
      *buf = *loc;

    *loc = NULL;
    pad_table(table, --table->count);
  } else {
    if (*buf)
      *buf = NULL;
  }

  return out;
}

/* miscellaneous utilities */
void pad_vec(Vec *vec, usize count) {
  usize cap = pad_array_size(count, vec->cap);

  if (cap != vec->cap) {
    vec->array = reallocate(vec->array, cap * sizeof(Val), vec->cap * sizeof(Val));
    vec->cap   = cap;
  }
}

void pad_bin(Bin *bin, usize count) {
  usize cap = pad_array_size(count, bin->cap);

  if (cap != bin->cap) {
    usize elsize = bin_elsize(bin);
    bin->array   = reallocate(bin->array, cap * elsize, bin->cap * elsize);
    bin->cap     = cap;
  }
}

void pad_buf(Bin *bin, usize count) {
  pad_bin(bin, count+1);
}

void pad_table(Table *table, usize count) {
  usize cap = pad_table_size(count, table->cap);

  if (cap != table->cap) {
    rehash_table(table, cap);
    table->cap = cap;
  }
}
