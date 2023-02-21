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
List   EmptyList = { .arity=0, .head=NUL, .tail=&EmptyList };

Bin    EmptyString;

// API ------------------------------------------------------------------------
// common utilities -----------------------------------------------------------
// lifetime & memory management -----------------------------------------------
void init_obj(void* self, Type type, flags fl) {
  Obj* obj     = self;
  obj->next    = Heap.live;
  Heap.live    = self;

  if (has_flag(self, INITIALIZED))
    return;
  
  obj->type   = type;
  obj->flags  = fl;
  obj->black  = false;
  obj->gray   = true;
}

void mark_obj(void* self) {
  if (!self)
    return;

  Obj* obj = self;

  if (obj->black)
    return;

  obj->black = true;

  if (mtable(self)->trace)
    push_objs(&Vm.heap.grays, self);

  else
    obj->gray = false;
}

void destruct_obj(void* self) {
  if (!self)
    return;

  assert(!has_flag(self, NOFREE));

  usize freed = mtable(self)->size;

  if (mtable(self)->destruct)
     freed += mtable(self)->destruct(self);

  if (has_flag(self, NOFREE))
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

  return construct(SYM, 1, strlen(name)+1, name);
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
  self->constant = UNDEFINED;
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
extern void init_chunk(Chunk* chunk);

Func* new_func(flags fl, uint arity, Sym* name, Mtable* mtable, void* func) {
  Func* out = mk_func(!!(fl&USER), func);

  init_func(out, fl, arity, name, mtable, func);

  return out;
}

Func* mk_func(bool userp, void* func) {
  return construct(FUNC, 1, sizeof(Chunk) * userp, func);
}

void  init_func(Func* self, flags fl, uint arity, Sym* name, Mtable* mtable, void* func) {
  init_obj(self, FUNC, fl);

  self->arity  = arity;
  self->name   = name;
  self->mtable = mtable;

  if (!!(fl&USER)) {
    self->func = &self[1];

    if (func == NULL)
      init_chunk(self->func);
  } else {
    assert(func);
    self->func = func;
  }

  set_flag(self, INITIALIZED);
}

// list api -------------------------------------------------------------------
List* new_list(Val head, List* tail) {
  List* out = mk_list();

  init_list(out, 0, head, tail);

  return out;
}

List* mk_list(void) {
  return construct(LIST, 1, 0, NULL);
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

// bin api --------------------------------------------------------------------
Bin* new_bin(bool encoded, uint n, void* data) {
  if (encoded && n == 0)
    return &EmptyString;

  Bin* out = mk_bin();

  init_bin(out, encoded, n, data);

  return out;
}

Bin* mk_bin(void) {
  return construct(BIN, 1, 0, NULL);
}

void init_bin(Bin* self, flags fl, uint n, void* data) {
  init_obj((Obj*)self, BIN, fl);

  uint c = pad_alist_size(n+has_flag(self, ENCODED), 0);

  self->array = allocate(c, 1, 0);
  self->count = n;
  self->cap   = c;

  if (data)
    memcpy(self->array, data, n);

  set_flag(self, INITIALIZED);
}

void resize_bin(Bin* self, uint n) {
  uint cap = pad_alist_size(n+has_flag(self, ENCODED), self->cap);

  if (cap != self->cap) {
    self->array = reallocate(self->array, cap, self->cap, 1, 0);
    self->cap   = cap;
  }

  self->count = n;
}

ubyte bin_get(Bin* self, uint n) {
  assert(n < self->count);
  return ((ubyte*)self->array)[n];
}

ubyte bin_set(Bin* self, uint n, ubyte xx) {
  assert(n < self->count);
  ((ubyte*)self->array)[n] = xx;

  return xx;
}

uint bin_write(Bin* self, uint n, void* data) {
  uint off = self->count;
  
  resize_bin(self, self->count+n);

  memcpy(self->array+off, data, n);

  return self->count;
}

// table api ------------------------------------------------------------------
static void rehash_table(Table* self, uint newc) {
  int* newo = allocate(newc, sizeof(int), -1), *oldo = self->ord;
  uint mask = (newc-1), oldc = self->cap;

  for (uint i=0, n=0; i < self->cap && n < self->count; ) {
    if (oldo[i] < 0)
      continue;

    uhash h  = hash(self->table[i].key, !!(self->flags&EQUAL));
    uint idx = h & mask;

    while (newo[idx] > 0)
      idx = (idx+1) & mask;

    newo[idx] = oldo[i];
  }

  deallocate(self->ord, self->cap, sizeof(int));
  self->ord   = newo;
  self->table = reallocate(self->table, newc, oldc, sizeof(Entry), NOTFOUND);
}

Table* new_table(bool eql, uint n, Val* args) {
  Table* out = mk_table();
  init_table(out, eql, n, args);
  return out;
}

Table* mk_table(void) {
  return construct(TABLE, 1, 0, 0);
}

void init_table(Table* self, flags fl, uint n, Val* args) {
  static Val initbuf[2] = { NOTFOUND, NOTFOUND };
  
  init_obj((Obj*)self, TABLE, fl);

  self->count  = 0;
  self->cap    = pad_table_size(n, 0);
  self->ord    = allocate(self->cap, sizeof(int), -1);
  self->table  = allocate(self->cap, sizeof(Entry), (uintptr_t)&initbuf);

  if (args)
    for (uint i=0; i<n*2; i+= 2)
      table_set(self, args[i], args[i+1]);

  set_flag(self, INITIALIZED);
}

void resize_table(Table* self, uint n) {
  uint c = pad_table_size(n, self->cap);

  if (c != self->cap) {
    rehash_table(self, c);

    self->cap = c;
  }
}

int* table_lookup(Table* self, Val key) {
  uhash h = hash(key, self->flags&EQUAL);
  uint  m = self->cap-1;
  uint  i = h & m;
  bool (*cmp)(Val x, Val y) = self->flags&EQUAL ? equal : same;
  int *o;

  while (*(o=self->ord+i) > 0) {
    if (cmp(key, self->table[*o].key))
      break;

    i = (i+1) & m;
  }

  return o;
}

Val* table_nth(Table* self, uint n) {
  assert(n <= self->count);
  return &self->table[n].key;
}

Val table_get(Table* self, Val key) {
  int* o = table_lookup(self, key);

  if (*o == -1)
    return NOTFOUND;

  return table_nth(self, *o)[1];
}

Val table_set(Table* self, Val key, Val val) {
  if (has_flag(self, INITIALIZED))
    resize_table(self, self->count+1);

  bool added = false;
  int *o     = table_lookup(self, key);

  if (*o == -1) {
    *o = self->count++;
    added    = true;
  }

  Val* spc = table_nth(self, *o);
  spc[1]   = val;

  if (added)
    spc[0] = key;

  return val;
}

Val table_del(Table* self, Val key) {
  int *o = table_lookup(self, key);

  if (*o == -1)
    return NOTFOUND;

  Val* spc = table_nth(self, *o);
  Val  out = spc[1];
  spc[0]   = NOTFOUND;
  spc[1]   = NOTFOUND;

  resize_table(self, --self->count);

  return out;
}

// objs array API -------------------------------------------------------------
void init_objs(Objs* objs) {
  objs->array = NULL;
  objs->count = 0;
  objs->cap   = pad_alist_size(0, 0);
}

void free_objs(Objs* objs) {
  deallocate(objs->array, objs->count, sizeof(Obj));
  init_objs(objs);
}

void resize_objs(Objs* objs, uint n) {
  uint c = pad_alist_size(n, objs->cap);

  if (c != objs->cap) {
    objs->array = reallocate(objs->array, objs->count, c, sizeof(Obj), NOTUSED);
    objs->cap   = c;
  }
}

uint push_objs(Objs* objs, Obj* obj) {
  resize_objs(objs, objs->count+1);

  objs->array[objs->count] = obj;

  return objs->count++;
}

Obj* pop_objs(Objs* objs) {
  assert(objs->count > 0);
  assert(objs->array != NULL);

  Obj* out = objs->array[--objs->count];

  resize_objs(objs, objs->count);

  return out;
}

// initialization -------------------------------------------------------------
extern uhash EmptyListHash, EmptyStringHash;

void object_init(void) {
  // initialize globals -------------------------------------------------------
  SymbolTable = NULL;

  init_list(&EmptyList, FROZEN|NOFREE, tag(&EmptyList), &EmptyList);
  init_bin(&EmptyString, FROZEN|NOFREE|ENCODED, 0, "");

  EmptyStringHash = mix_hashes(2, hash_ptr(&EmptyString), hash_uint(BIN));
  EmptyListHash   = mix_hashes(2, hash_ptr(&EmptyList), hash_uint(LIST));
}
