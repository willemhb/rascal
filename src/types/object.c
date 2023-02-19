#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "types/object.h"

#include "runtime/memory.h"

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

// initialization -------------------------------------------------------------
void object_init(void) {
  // initialize globals -------------------------------------------------------
  init_obj((Obj*)&EmptyList, LIST_TYPE);
  EmptyList.head = tag(&EmptyList);
  EmptyList.tail = &EmptyList;

  // initialize types ----------------------------------------------------------
  extern void trace_list(void* self);
  extern void print_list(Val x, void* state);
  extern Val  hash_list(Val x, void* state);
  extern Val  equal_lists(Val x, Val y, void* state);
  extern Val  compare_lists(Val x, Val y, void* state);

  MetaTables[LIST_TYPE] = (Mtable) {
    .name       =intern("list"),
    .type       =LIST_TYPE,
    .kind       =DATA_KIND,
    .type_hash =hash_uint(LIST_TYPE),
    .size      =sizeof(List),
    .trace     =trace_list,
    .print     =print_list,
    .hash      =hash_list,
    .equal     =equal_lists,
    .compare   =compare_lists
  };

  extern void  trace_sym(void* self);
  extern usize destruct_sym(void* self);
  extern void  print_sym(Val x, void* state);
  extern Val   hash_sym(Val x, void* state);
  extern Val   compare_syms(Val x, Val y, void* state);

  MetaTables[SYM_TYPE] = (Mtable) {
    .name      = intern("sym"),
    .type      = SYM_TYPE,
    .kind      = DATA_KIND,
    .type_hash = hash_uint(SYM_TYPE),
    .size      = sizeof(Sym),
    .trace     = trace_sym,
    .destruct  = destruct_sym,
    .print     = print_sym,
    .hash      = hash_sym,
    .compare   = compare_syms
  };

  extern void  trace_func(void* self);
  extern usize destruct_func(void* self);
  extern void  print_func(Val x, void* state);
  extern Val   hash_func(Val x, void* state);
  extern Val   equal_funcs(Val x, Val y, void* state);
  extern Val   compare_funcs(Val x, Val y, void* state);

  MetaTables[FUNC_TYPE] = (Mtable) {
    .name       = intern("func"),
    .type       = FUNC_TYPE,
    .kind       = DATA_KIND,
    .type_hash  = hash_uint(FUNC_TYPE),
    .size       = sizeof(Func),
    .trace      = trace_func,
    .destruct   = destruct_func,
    .print      = print_func,
    .hash       = hash_func,
    .equal      = equal_funcs,
    .compare    = compare_funcs
  };
}
