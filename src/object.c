#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "object.h"

#include "memory.h"

#include "util/hash.h"

/* globals */
usize SymbolCounter = 1;
Sym*  SymbolTable = NULL;
List  EmptyList;

/* API */
// flag getters & setters -----------------------------------------------------
bool has_flag(Obj* o, flags fl) {
  return !!(o->flags&fl);
}

bool set_flag(Obj* o, flags fl) {
  bool out = !has_flag(o, fl);
  o->flags |= fl;
  return out;
}

bool clear_flag(Obj* o, flags fl) {
  bool out  = has_flag(o, fl);
  o->flags &= ~fl;
  return out;
}

// lifetime & memory management -----------------------------------------------
void init_obj(Obj* self, Type type) {
  self->next   = Heap.live;
  Heap.live    = self;
  self->type   = type;
  self->flags  = 0;
  self->nofree = false;
  self->black  = false;
  self->gray   = true;
}

void mark_obj(Obj* self) {
  if (!self)
    return;

  if (self->black)
    return;

  self->black = true;

  if (mtable(self)->trace)
    add_gray(self);

  else
    self->gray = false;
}

void destruct_obj(Obj* self) {
  usize freed = mtable(self)->size;

  if (mtable(self)->destruct)
     freed += mtable(self)->destruct(self);

  if (!self->nofree)
    deallocate(self, freed);
}

// symbol api -----------------------------------------------------------------
uhash hash_symbol_data(char* name, uidno idno) {
  static uhash th = 0;

  if (th == 0)
    th = hash_uint(SYM_TYPE);

  uhash ih = hash_uint(idno);
  uhash nh = hash_str(name);

  return mix_hashes(3, nh, ih, th);
}

void init_sym(Sym* self, bool generated, char* name) {
  
  init_obj(&self->obj, SYM_TYPE);

  self->name      = duplicate(name, strlen(name));
  self->idno      = SymbolCounter++;
  self->hash      = hash_symbol_data(self->name, self->idno);
  self->obj.flags = (!generated)*INTERNED | (*name == ':')*LITERAL;
  self->constant  = UNDEFINED;
  self->left      = NULL;
  self->right     = NULL;
}

Sym* new_sym(bool generated, char* name) {
  Sym* out = construct(SYM_TYPE, 1, 0);

  init_sym(out, generated, name);

  return out;
}

static Sym** find_in_symbol_table(char* name) {
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

Sym* intern(char* name) {
  Sym** node = find_in_symbol_table(name);

  if (*node == NULL)
    *node = new_sym(false, name);

  return *node;
}


Val gensym(Val name) {
  char* text = as_text(name);

  if (text == NULL)
    text = "symbol";

  return tag(new_sym(true, text));
}

Val symbol(Val name) {
  if (is_sym(name))
    return name;
  
  char* text = as_text(name);

  return tag(intern(text));
}

Val keyword(Val name) {
  if (is_sym(name) && is_keyword(as_sym(name)))
    return name;

  char *text = as_text(name);

  char buf[strlen(text)+2];

  buf[0] = ':';
  strcpy(buf+1, text);

  return tag(intern(buf));
}

void set_const(Sym* sym, Val val) {
  assert(sym->constant == UNDEFINED);

  sym->constant = val;
}

bool is_gensym(Sym* sym) {
  return !has_flag((Obj*)sym, INTERNED);
}

bool is_literal(Sym* sym) {
  return has_flag((Obj*)sym, LITERAL);
}

bool is_keyword(Sym* sym) {
  return sym->name[0] == ':';
}

// list api -------------------------------------------------------------------
void init_list(List* self, Val head, List* tail) {
  init_obj((Obj*)self, LIST_TYPE);

  self->head      = head;
  self->tail      = tail;
  self->len       = tail->len+1;
}

List* cons(Val head, List* tail) {
  List* out = construct(LIST_TYPE, 1, 0);
  init_list(out, head, tail);
  return out;
}

List* list(uint n, ...) {
  if (n == 0)
    return &EmptyList;

  Val buf[n];

  va_list va; va_start(va, n);

  for (uint i=0; i<n; i++)
    buf[i] = va_arg(va, Val);

  va_end(va);

  List* out  = construct(LIST_TYPE, n, 0);
  List* last = &EmptyList;

  for (uint i=n; i>0; i--) {
    init_list(out+n-1, buf[n-1], last);
    last = out+n-1;
  }

  return out;
}

List* list_assc(List* ls, Val k) {
  while (ls->len) {
    if (ls->head == k)
      break;

    ls = ls->tail;
  }

  return ls;
}

Val list_nth(List* ls, uint n) {
  assert(n < ls->len);

  while (n--)
    ls = ls->tail;

  return ls->head;
}

// func api -------------------------------------------------------------------
extern Bin*   new_bin(bool encoded, uint n, void* data);
extern Table* new_table(bool eql, int n, Val* args);

void init_func(Func* self, bool native, uint arity, Val name, Mtable* type, void* func) {
  init_obj((Obj*)self, FUNC_TYPE);

  self->arity  = arity;
  self->user   = !native;
  self->native = native;
  self->name   = as_sym(symbol(name));
  self->type   = type;

  if (native) {
    self->func.native = func;
  } else {
    self->func.user   = ((void*)self) + sizeof(Func);

    if (func) {
      memcpy(self->func.user, func, sizeof(Chunk));
    } else {
      self->func.user->ns    = &EmptyList;
      self->func.user->env   = NULL;
      self->func.user->code  = new_bin(false, 0, NULL);
      self->func.user->consts= new_table(false, 0, NULL);
    }
  }
}

Func* new_func(bool native, uint arity, Val name, Mtable* type, void* func) {
  usize extra = native ? 0 : sizeof(Chunk);
  Func* out   = construct(FUNC_TYPE, 1, extra);

  init_func(out, native, arity, name, type, func);

  return out;
}

bool is_type(Func* f) {
  return f->type != NULL;
}

bool is_closure(Func* f) {
  return f->user && f->func.user->env != NULL;
}

// binary type ----------------------------------------------------------------
uhash hash_bin_data(uint n, void* data) {
  static uhash th = 0;

  if (th == 0)
    th = hash_uint(BIN_TYPE);

  uhash dh = hash_mem(data, n);

  return mix_hashes(2, dh, th);
}

void init_bin(Bin* self, bool encoded, uint n, void* data) {
  init_obj((Obj*)self, BIN_TYPE);
  
  self->obj.flags = ENCODED*encoded;
  self->count     = n;
  self->cap       = pad_alist_size(n+encoded, 0);
  self->array     = allocate(self->cap);

  if (data)
    memcpy(self->array, data, self->cap);

  if (data && encoded)
    self->hash = hash_bin_data(n, data);
}

Bin* new_bin(bool encoded, uint n, void* data) {
  Bin* out = construct(BIN_TYPE, 1, 0);

  init_bin(out, encoded, n, data);

  return out;
}

Bin* bytes(uint n, ubyte* bytes) {
  return new_bin(false, n, bytes);
}

Bin* string(char* chars) {
  return new_bin(true, strlen(chars), chars);
}

Bin* bytecode(uint n, uint16* code) {
  return new_bin(false, n*2, code);
}

bool is_string(Bin* bin) {
  return has_flag((Obj*)bin, ENCODED);
}

void resize_bin(Bin* self, uint n) {
  uint cap = pad_alist_size(n+is_string(self), self->cap);

  if (cap != self->cap) {
    self->array = reallocate(self->array, cap, self->cap);
    self->cap   = cap;
  }

  self->count = n;

  if (is_string(self))
    self->hash = 0;
}

void* bin_peep(Bin* self, int i) {
  if (i < 0)
    i += self->count;

  assert(i >= 0 && (uint)i < self->count);
  return self->array + i;
}

ubyte bin_ref(Bin* self, int i) {
  return *(ubyte*)bin_peep(self, i);
}

ubyte bin_set(Bin* self, int i, ubyte byte) {
  return (*(ubyte*)bin_peep(self, i) = byte);
}

usize bin_write(Bin* self, usize n, void* data) {
  usize off = self->count;

  resize_bin(self, self->count+n);

  memcpy(self->array+off, data, n);

  return self->count;
}

static void rehash_table(Table* self, uint newc) {
  int* newo = callocate(newc, sizeof(int), -1), *oldo = self->ord;
  uint mask = (newc-1), oldc = self->cap;

  for (uint i=0, n=0; i < self->cap && n < self->count; ) {
    if (oldo[i] < 0)
      continue;

    uhash h  = hash(self->table[i].key, self->eql);
    uint idx = h & mask;

    while (newo[idx] > 0)
      idx = (idx+1) & mask;

    newo[idx] = oldo[i];
  }

  cdeallocate(self->ord, self->cap, sizeof(int));
  self->ord   = newo;
  self->table = creallocate(self->table, newc*2, oldc*2, sizeof(Val), NOTFOUND);
  self->cap   = newc;
}

/* API */
Table* new_table(bool eql, uint n, Val* args) {
  Table* out = construct(TABLE_TYPE, 1, 0);
  init_table(out, eql, n, args);
  return out;
}

void init_table(Table* self, bool eql, uint n, Val* args) {
  init_obj((Obj*)self, TABLE_TYPE);

  self->eql    = eql;
  self->count  = 0;
  self->cap    = pad_table_size(n, 0);
  self->ord    = callocate(self->cap, sizeof(int), -1);
  self->table  = callocate(self->cap*2, sizeof(Val), NOTFOUND);

  if (args)
    for (uint i=0; i<n*2; i+= 2)
      table_set(self, args[i], args[i+1]);

  self->init = true;
}

void resize_table(Table* self, uint n) {
  uint c = pad_table_size(n, self->cap);

  if (c != self->cap)
    rehash_table(self, c);
}

int* table_lookup(Table* self, Val key) {
  uhash h = hash(key, self->eql);
  uint  m = self->cap-1;
  uint  i = h & m;
  bool (*cmp)(Val x, Val y) = self->eql ? equal : same;
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
  if (self->init)
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

// initialization -------------------------------------------------------------
void object_init(void) {}
