#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "data.h"
#include "runtime.h"
#include "lang.h"
#include "collection.h"
#include "util.h"

// forward declarations
void print_list(Port* ios, Expr x);
void print_num(Port* ios, Expr x);


bool egal_lists(Expr x, Expr y);

void trace_list(void* ptr);

// Globals
ExpTypeInfo Types[NUM_TYPES] = {
  [EXP_LIST] = {
    .type     = EXP_LIST,
    .name     = "list",
    .obsize   = sizeof(List),
    .print_fn = print_list,
    .egal_fn  = egal_lists,
    .trace_fn = trace_list
  },

  [EXP_NUM] = {
    .type     = EXP_NUM,
    .name     = "num",
    .obsize   = 0,
    .print_fn = print_num
  }
};

// utility array APIs
void trace_exprs(Exprs* xs) {
  for ( int i=0; i < xs->count; i++ )
    mark_exp(xs->vals[i]);
}

void trace_objs(Objs* os) {
  for ( int i=0; i < os->count; i++ )
    mark_obj(os->vals[i]);
}

// expression APIs

// object API
void* as_obj(Expr x) {
  return (void*)(x & XVMSK);
}

Expr tag_obj(void* o) {
  return ((Expr)o) | OBJ_T;
}

void* mk_obj(ExpType type, flags_t flags) {
  Obj* out = allocate(true, Types[type].obsize);

  out->type     = type;
  out->bfields  = flags | FL_GRAY;
  out->heap     = Heap;
  Heap          = out;

  return out;
}

void* clone_obj(void* ptr) {
  assert(ptr != NULL);

  Obj* obj = ptr;
  ExpTypeInfo* info = &Types[obj->type];
  Obj* out = duplicate(true, info->obsize, obj);

  if ( info->clone_fn )
    info->clone_fn(out);

  return out;
}

void mark_obj(void* ptr) {
  Obj* obj = ptr;

  if ( obj != NULL ) {
    if ( obj->black == false ) {
      obj->black = true;

      ExpTypeInfo* info = &Types[obj->type];

      if ( info->trace_fn )
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
    ExpTypeInfo* info = &Types[obj->type];

    if ( info->free_fn )
      info->free_fn(obj);

    release(obj, info->obsize);
  }
}

// miscellaneous APIs

// lifetime methods -----------------------------------------------------------

// symbol API

// string API


// list API
static List* new_lists(size_t n) {
  assert(n > 0);

  size_t nb = (n+1) * sizeof(List);
  List* xs  = allocate(true, nb);

  // initialize terminal empty list
  for ( size_t i=0; i < n; i++ ) {
    List* cell = &xs[i];

    // initialize the list object
    cell->heap   = (Obj*)(&cell+1);
    cell->type   = EXP_LIST;
    cell->black  = false;
    cell->gray   = true;
    cell->head   = NUL;
    cell->tail   = cell + 1;
    cell->count  = n - i;
  }

  // handle the terminal empty list specially
  List* cell  = &xs[n];
  cell->heap   = Heap;
  cell->type   = EXP_LIST;
  cell->black  = false;
  cell->gray   = true;
  cell->head   = NUL;
  cell->tail   = NULL;
  cell->count  = 0;

  // add it all to the heap
  Heap = (Obj*)xs;

  return xs;
}

List* as_list_s(char* f, Expr x) {
  require_argtype(f, EXP_LIST, x);

  return as_list(x);
}

List* empty_list(void) {
  List* l = mk_obj(EXP_LIST, 0);

  l->head  = NUL;
  l->tail  = NULL;
  l->count = 0;

  return l;
}

List* mk_list(size_t n, Expr* xs) {
  List* l;

  if ( n == 0 )
    l = empty_list();

  else {
    l = new_lists(n);

    for ( size_t i=0; i<n; i++ )
      l[i].head = xs[i];
  }

  return l;
}

List* cons(Expr hd, List* tl) {
  assert(tl != NULL);
  preserve(1, tag_obj(tl));

  List* l  = mk_obj(EXP_LIST, 0);
  l->head  = hd;
  l->tail  = tl;
  l->count = tl->count+1;

  return l;
}

Expr list_ref(List* xs, int n) {
  assert(n >= 0);
  assert(n < (int)xs->count-1);

  while ( n-- )
    xs = xs->tail;

  return xs->head;
}

void print_list(Port* ios, Expr x) {
        pprintf(ios, "(");

      List* xs = as_list(x);

      while ( xs->count > 0 ) {
        print_exp(ios, xs->head);

        if ( xs->count > 1 )
          pprintf(ios, " ");

        xs = xs->tail;
      }

      pprintf(ios, ")");
}

bool egal_lists(Expr x, Expr y) {
  List* xs = as_list(x), * ys = as_list(y);

  bool out = xs->count == ys->count;

  while ( out && xs->count > 0 ) {
    x   = xs->head;
    y   = ys->head;
    out = egal_exps(x, y);

    if ( out ) {
      xs = xs->tail;
      ys = ys->tail;
    }
  }

  return out;
}

void trace_list(void* ptr) {
  List* xs = ptr;

  if ( xs->count ) {
    mark_exp(xs->head);
    mark_obj(xs->tail);
  }
}

// number APIs
Num as_num_s(char* f, Expr x) {
  ExpType t = exp_type(x);
  require(t == EXP_NUM, "%s wanted type num, got %s", f, Types[t].name);
  return as_num(x);
}

Num as_num(Expr x) {
  Val v = { .expr = x };

  return v.num;
}

Expr tag_num(Num n) {
  Val v = { .num = n };

  return v.expr;
}

// shortcut for tagging pointers and small integers safely
uintptr_t as_fix(Expr x) {
  return x & XVMSK;
}

Expr tag_fix(uintptr_t i) {
  return ( i & XVMSK) | FIX_T;
}

void* as_ptr(Expr x) {
  return (void*)(x & XVMSK);
}

Expr tag_ptr(void* ptr) {
  return ((uintptr_t)ptr) | FIX_T;
}

void print_num(Port* ios, Expr x) {
  pprintf(ios, "%g", as_num(x));
}

// glyph APIs
