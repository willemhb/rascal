#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "object.h"
#include "compare.h"
#include "runtime.h"

#include "util/hash.h"
#include "util/collections.h"

/* globals */
uint32 SymbolCounter = 1;
Sym*   SymbolTable = NULL;

List   EmptyList = {
  .next=NULL,
  .type=LIST,
  .flags=FROZEN|NOFREE|INITIALIZED,
  .arity=0,
  .head=NUL,
  .tail=&EmptyList
};

Str EmptyString = {
  .next=NULL,
  .type=STR,
  .flags=FROZEN|NOFREE|ENCODED|INITIALIZED,
  .arity=0,
  .hash=0,
  .chars={ '\0' }
};

Tuple EmptyTuple = {
  .next=NULL,
  .type=TUPLE,
  .flags=FROZEN|NOFREE|INITIALIZED,
  .arity=0
};

// API ------------------------------------------------------------------------
// common utilities -----------------------------------------------------------
// lifetime & memory management -----------------------------------------------
void init_obj(void* self, Type type, flags fl) {
  if (has_flag(self, INITIALIZED))
    return;

  Obj* obj    = self;
  obj->type   = type;
  obj->flags  = fl;

  register_obj(obj);
}

void mark_obj(void* self) {
  if (!self)
    return;

  Obj* obj = self;

  if (obj->black)
    return;

  obj->black = true;

  if (mtable(self)->trace)
    objs_push(&Vm.heap.grays, self);

  else
    obj->gray = false;
}

void destruct_obj(void* self) {
  if (!self)
    return;

  usize freed = mtable(self)->size;

  if (mtable(self)->destruct)
     freed += mtable(self)->destruct(self);

  if (!has_flag(self, NOFREE))
    deallocate(self, freed, 1);
}

bool has_flag(void* self, flags fl) {
  return !!(((Obj*)self)->flags&fl);
}

bool set_flag(void* self, flags fl) {
  bool out = has_flag(self, fl);

  ((Obj*)self)->flags |= fl;

  return out;
}

bool del_flag(void* self, flags fl) {
  bool out = has_flag(self, fl);
  
  ((Obj*)self)->flags &= ~fl;

  return out;
}

// symbol api -----------------------------------------------------------------
static Sym** find_in_symbol_table(char* name) {
  assert(name != NULL);
  assert(*name != '\0');

  Sym** node = &SymbolTable;

  while (*node) {
    int o = strcmp(name, (*node)->name);

    if (o < 0)
      node = &(*node)->left;

    else if (o > 0)
      node = &(*node)->right;

    else
      break;
  }

  return node;
}

Sym* mk_sym(char* name) {
  assert(name != NULL && *name != '\0');

  return construct(SYM, 1, strlen(name)+1, sizeof(char), (uintptr_t)name, true);
}

Sym* new_sym(bool genp, char* name) {
  assert(SymbolCounter < UINT32_MAX);
  assert(name != NULL && *name != '\0');

  Sym* out = mk_sym(name);
  init_sym(out, genp, name);

  return out;
}

void init_sym(Sym* self, flags fl, char* name) {
  assert(SymbolCounter < UINT32_MAX);
  assert(name != NULL && *name != '\0');

  fl |= LITERAL*(*name == ':');
  
  init_obj(self, SYM, fl);

  self->name     = (char*)&self[1];
  self->arity    = SymbolCounter++;
  self->hash     = mix_hashes(3, hash_str(name), hash_uint(self->arity), MetaTables[SYM].type_hash);
  self->bind     = UNDEFINED;
  self->left     = NULL;
  self->right    = NULL;

  set_flag(self, INITIALIZED);
}

Sym* get_sym(bool genp, char* name) {
  if (name == NULL || *name == '\0')
    name = "symbol";

  Sym* out;

  if (!genp) {
    Sym** node = find_in_symbol_table(name);

    if (*node == NULL)
      *node = new_sym(false, name);

    out = *node;
  } else {
    out = new_sym(true, name);
  }

  return out;
}

// func api -------------------------------------------------------------------
Func* new_func(flags fl, uint arity, Sym* name, Mtable* mtable, void* func) {
  Func* out = mk_func(!!(fl&USER), func);

  init_func(out, fl, arity, name, mtable, func);

  return out;
}

Func* mk_func(bool userp, void* func) {
  return construct(FUNC, 1, sizeof(Chunk) * userp, sizeof(ubyte), (uintptr_t)func, true);
}

void  init_func(Func* self, flags fl, uint arity, Sym* name, Mtable* mtable, void* func) {
  init_obj(self, FUNC, fl);

  self->arity  = arity;
  self->name   = name;
  self->mtable = mtable;

  if ( flagp(fl, USER) )
    self->func = &self[1];

  else
    self->func = func;

  set_flag(self, INITIALIZED);
}

// bin api --------------------------------------------------------------------
Bin* new_bin(flags fl, uint n, void* data) {
  Bin* out = mk_bin();

  init_bin(out, fl, n, data);

  return out;
}

Bin* mk_bin(void) {
  return construct(BIN, 1, 0, 0, 0, false);
}

void init_bin(Bin* self, flags fl, uint n, void* data) {
  bool isencoded = flagp(fl, ENCODED);
  
  init_obj((Obj*)self, BIN, fl);

  self->count = 0;
  self->cap   = pad_stack_size(n + isencoded, 0);
  self->array = allocate(self->cap, sizeof(ubyte), 0);

  if (data) {
    self->count = n;
    memcpy(self->array, data, n*sizeof(ubyte));
    set_flag(self, INITIALIZED);
  }
}

void resize_bin(Bin* self, uint n) {
  if (has_flag(self, INITIALIZED)) {
    uint cap = pad_stack_size(n+has_flag(self, ENCODED), self->cap);

    if (cap != self->cap) {
      self->array = reallocate(self->array, cap, self->cap, 1, 0);
      self->cap   = cap;
    }
  }

  self->count = n;
}

uint bin_write(Bin* self, uint n, void* data) {
  uint off = self->count;

  resize_bin(self, self->count+n);

  memcpy(self->array+off, data, n);

  return self->count;
}

// str api --------------------------------------------------------------------
Str* new_str(flags fl, uint n, char* data) {
  if (n == 0)
    return &EmptyString;
  
  Str* out = mk_str(n);

  init_str(out, fl, n, data);

  return out;
}

Str* mk_str(uint n) {
  assert(n > 0);

  return construct(STR, 1, n+1, sizeof(char), '\0', false);
}

void init_str(Str* self, flags fl, uint n, char* data) {
  init_obj(self, STR, fl|ENCODED|FROZEN);

  self->arity = n;
  self->hash  = 0;

  if (data) {
    strcpy(self->chars, data);
    set_flag(self, INITIALIZED);
  }
}

// list api -------------------------------------------------------------------
List* new_list(Val head, List* tail) {
  List* out = mk_list();

  init_list(out, 0, head, tail);

  return out;
}

List* mk_list(void) {
  return construct(LIST, 1, 0, 0, 0ul, false);
}

void init_list(List* self, flags fl, Val head, List* tail) {
  if (tail == NULL)
    tail = &EmptyList;

  init_obj(self, LIST, fl);

  self->head = head;
  self->tail = tail;
  self->arity= tail->arity+1;
  set_flag(self, INITIALIZED);
}

Val list_nth(List* list, uint n) {
  assert(n < list->arity);

  while (n--)
    list = list->tail;

  return list->head;
}

List* list_assoc(List* list, Val k) {
  for (;list->arity; list=list->tail)
    if (list->head == k)
      break;

  return list;
}

// vec api --------------------------------------------------------------------
Vec* new_vec(uint n, Val* args) {
  Vec* out = mk_vec();

  init_vec(out, 0, n, args);

  return out;
}

Vec* mk_vec(void) {
  return construct(VEC, 1, 0, 0, 0ul, false);
}

void init_vec(Vec* self, flags fl, uint n, Val* args) {
  init_obj(self, VEC, fl);

  uint c = pad_alist_size(0, n, 0);

  self->array = allocate(c, sizeof(Val), NOTUSED);
  self->cap   = c;

  if (args) {
    self->count = n;
    memcpy(self->array, args, n*sizeof(Val));
    set_flag(self, INITIALIZED);
  } else {
    self->count = 0;
  }
}

uint vec_push(Vec* vec, Val v) {
  return vals_push((Vals*)&vec->array, v);
}

Val vec_pop(Vec *vec) {
  return vals_pop((Vals*)&vec->array);
}

void resize_vec(Vec *vec, uint n) {
  resize_vals((Vals*)&vec->array, n);
}

// tuple api ------------------------------------------------------------------
Tuple* new_tuple(uint n, Val* args) {
  if (n == 0)
    return &EmptyTuple;

  Tuple* out = mk_tuple(n);

  init_tuple(out, 0, n, args);

  return out;
}

Tuple* mk_tuple(uint n) {
  assert(n);

  return construct(TUPLE, 1, n, sizeof(Val), NOTUSED, false);
}

void init_tuple(Tuple* self, flags fl, uint n, Val* args) {
  init_obj(self, TUPLE, fl|FROZEN);

  self->arity = n;

  if (args) {
    memcpy(self->slots, args, n*sizeof(Val));
    set_flag(self, INITIALIZED);
  }
}

// table api ------------------------------------------------------------------
static void rehash_table(Table* self, uint newc) {
  Tuple** newt = allocate(newc, sizeof(Tuple*), (uintptr_t)&EmptyTuple);
  uint mask = (newc-1), oldc = self->cap;

  for (uint i=0, n=0; i < self->cap && n < self->count; ) {
    Tuple *kv = newt[i];

    if (kv == NULL)
      continue;

    uhash h  = hash(kv->slots[KEY], has_flag(self, EQUAL));
    uint idx = h & mask;

    while (newt[idx] != &EmptyTuple)
      idx = (idx+1) & mask;

    newt[idx] = kv;
  }

  deallocate(self->table, self->cap, sizeof(Tuple*));
  self->table = reallocate(self->table, newc, oldc, sizeof(Tuple*), 0ul);
}

static Tuple **find_in_table(Table* self, Val key) {
  bool equalp               = has_flag(self, EQUAL);
  bool (*cmp)(Val x, Val y) = equalp ? equal : same;
  uhash h                   = hash(key, equalp);
  uint mask                 = self->cap - 1;
  uint idx                  = h & mask;

  while (self->table[idx]) {
    if (cmp(key, self->table[idx]->slots[KEY]))
      break;

    idx = (idx + 1) & mask;
  }

  return self->table+idx;
}

Table* new_table(flags fl, uint n, Val* args) {
  Table* out = mk_table();
  init_table(out, fl, n, args);
  return out;
}

Table* mk_table(void) {
  return construct(TABLE, 1, 0, 0, 0, false);
}

void init_table(Table* self, flags fl, uint n, Val* args) {
  assert(n % 2 == 0);

  init_obj((Obj*)self, TABLE, fl);

  self->count = 0;
  self->cap   = pad_table_size(n, 0);
  self->table = allocate(self->cap, sizeof(Tuple*), (uintptr_t)&EmptyTuple);

    if (args) {
      for (uint i=0; i<n*2; i+= 2)
	table_set(self, args[i], args[i+1]);

      set_flag(self, INITIALIZED);
    }
}

void resize_table(Table* self, uint n) {
  if (has_flag(self, INITIALIZED)) { 
    uint c = pad_table_size(n, self->cap);

    if (c != self->cap) {
      rehash_table(self, c);
      self->cap = c;
    }
  }
}

Val table_get(Table* self, Val key) {
  Tuple** location = find_in_table(self, key);

  if (*location == NULL)
    return NOTFOUND;

  return (*location)->slots[BIND];
}

Val table_set(Table* self, Val key, Val val) {
  resize_table(self, self->count+1);

  Val out          = NUL;
  Tuple** location = find_in_table(self, key);

  if (*location == NULL) {
    *location = mk_pair(key, NUL);
    out = tag((Real)self->count++);
  }

  (*location)->slots[BIND] = val;

  return out;
}

Val table_del(Table* self, Val key) {
  Tuple** location = find_in_table(self, key);

  if (*location == NULL)
    return NOTFOUND;

  Val out = tag(*location);

  *location = NULL;

  resize_table(self, --self->count);

  return out;
}

// objs array API -------------------------------------------------------------
void init_objs(Objs* objs) {
  objs->array = NULL;
  objs->count = 0;
  objs->cap   = pad_alist_size(0, 0, 0);
}

void free_objs(Objs* objs) {
  deallocate(objs->array, objs->count, sizeof(Obj));
  init_objs(objs);
}

void resize_objs(Objs* objs, uint n) {
  uint c = pad_alist_size(objs->count, n, objs->cap);

  if (c != objs->cap) {
    objs->array = reallocate(objs->array, objs->count, c, sizeof(Obj), NOTUSED);
    objs->cap   = c;
  }
}

uint objs_push(Objs* objs, Obj* obj) {
  resize_objs(objs, objs->count+1);

  objs->array[objs->count] = obj;

  return objs->count++;
}

Obj* objs_pop(Objs* objs) {
  assert(objs->count > 0);
  assert(objs->array != NULL);

  Obj* out = objs->array[--objs->count];

  resize_objs(objs, objs->count);

  return out;
}

int objs_search(Objs* objs, Obj* obj) {
  for (uint i=0; i<objs->count; i++) {
    if (objs->array[i] == obj)
      return i;
  }

  return -1;
}

// initialization -------------------------------------------------------------
void object_init(void) {
  // initialize globals -------------------------------------------------------
  SymbolTable = NULL;

  register_obj(&EmptyList);
  register_obj(&EmptyString);
  register_obj(&EmptyTuple);
}
