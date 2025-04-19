#include <string.h>
#include <stdlib.h>

#include "data.h"
#include "runtime.h"
#include "lang.h"

// forward declarations
void print_list(FILE* ios, Expr x);
void print_sym(FILE* ios, Expr x);
void print_num(FILE* ios, Expr x);
void print_nul(FILE* ios, Expr x);

void free_sym(void* ob);

// Globals
ExpTypeInfo Types[] = {
  [EXP_NUL] = {
    .type     = EXP_NUL,
    .name     = "nul",
    .obsize   = 0,
    .print_fn = print_nul
  },

  [EXP_EOS] = {
    .type   = EXP_EOS,
    .name   = "eos",
    .obsize = 0
  },

  [EXP_SYM] = {
    .type     = EXP_SYM,
    .name     = "sym",
    .obsize   = sizeof(Sym),
    .print_fn = print_sym,
    .free_fn  = free_sym
  },

  [EXP_LIST] = {
    .type     = EXP_LIST,
    .name     = "list",
    .obsize   = sizeof(List),
    .print_fn = print_list
  },

  [EXP_NUM] = {
    .type     = EXP_NUM,
    .name     = "num",
    .obsize   = 0,
    .print_fn = print_num
  }
};

// expression APIs
ExpType exp_type(Expr x) {
  ExpType t;

  switch ( x & XTMSK ) {
    case NUL  : t = EXP_NUL;         break;
    case EOS_T: t = EXP_EOS;         break;
    case OBJ  : t = head(x)->type;   break;
    default   : t = EXP_NUM;         break;
  }

  return t;
}

ExpTypeInfo* exp_info(Expr x) {
  return &Types[exp_type(x)];
}

// object API
void* as_obj(Expr x) {
  return (void*)(x & XVMSK);
}

Expr tag_obj(void* o) {
  return ((Expr)o) | OBJ;
}

void* mk_obj(ExpType type) {
  Obj* out = allocate(true, Types[type].obsize);

  out->type   = type;
  out->black  = false;
  out->gray   = true;
  out->heap   = Heap;
  Heap        = out;

  return out;
}

void free_obj(void* x) {
  Obj* obj = x;

  if ( obj ) {
    ExpTypeInfo* info = &Types[obj->type];

    if ( info->free_fn )
      info->free_fn(obj);

    release(obj, info->obsize);
  }
}

// miscellaneous APIs
void print_nul(FILE* ios, Expr x) {
  (void)x;
  fprintf(ios, "nul");
}

// symbol API
Sym* mk_sym(char* val) {
  Sym* s = mk_obj(EXP_SYM);
  s->val = strdup(val);

  return s;
}

bool sym_val_eql(Sym* s, char* v) {
  return strcmp(s->val, v) == 0;
}

void print_sym(FILE* ios, Expr x) {
  Sym* s = as_sym(x);

  fprintf(ios, "%s", s->val);
}

void free_sym(void* obj) {
  Sym* s = obj;

  free(s->val);
}

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
    cell->marked = false;
    cell->head   = NUL;
    cell->tail   = cell + 1;
    cell->count  = n - i;
  }

  // handle the terminal empty list specially
  List* cell  = &xs[n];
  cell->heap   = Heap;
  cell->type   = EXP_LIST;
  cell->marked = false;
  cell->head   = NUL;
  cell->tail   = NULL;
  cell->count  = 0;

  // add it all to the heap
  Heap = (Obj*)xs;

  return xs;
}

List* empty_list(void) {
  List* l = mk_obj(EXP_LIST);

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

void print_list(FILE* ios, Expr x) {
        fprintf(ios, "(");

      List* xs = as_list(x);

      while ( xs->count > 0 ) {
        print_exp(ios, xs->head);

        if ( xs->count > 1 )
          fprintf(ios, " ");

        xs = xs->tail;
      }

      fprintf(ios, ")");
}


// number APIs
Num as_num(Expr x) {
  Val v = { .expr = x };

  return v.num;
}

Expr tag_num(Num n) {
  Val v = { .num = n };

  return v.expr;
}

void print_num(FILE* ios, Expr x) {
  fprintf(ios, "%g", as_num(x));
}
