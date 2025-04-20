#include <string.h>
#include <stdlib.h>

#include "data.h"
#include "runtime.h"
#include "lang.h"
#include "collection.h"
#include "util.h"

// forward declarations
void print_list(FILE* ios, Expr x);
void print_sym(FILE* ios, Expr x);
void print_str(FILE* ios, Expr x);
void print_num(FILE* ios, Expr x);
void print_nul(FILE* ios, Expr x);
void print_none(FILE* ios, Expr x);

hash_t hash_sym(Expr x);
hash_t hash_str(Expr x);

void trace_env(void* ptr);
void trace_list(void* ptr);

void free_env(void* ptr);
void free_str(void* ptr);

// Globals
ExpTypeInfo Types[] = {
  [EXP_NONE] = {
    .type     = EXP_NONE,
    .name     = "none",
    .obsize   = 0,
    .print_fn = print_none
  },
  
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

  [EXP_ENV] = {
    .type     = EXP_ENV,
    .name     = "env",
    .obsize   = sizeof(Env),
    .trace_fn = trace_env,
    .free_fn  = free_env
  },

  [EXP_SYM] = {
    .type     = EXP_SYM,
    .name     = "sym",
    .obsize   = sizeof(Sym),
    .print_fn = print_sym,
    .hash_fn  = hash_sym
  },

  [EXP_STR] = {
    .type     = EXP_STR,
    .name     = "str",
    .obsize   = sizeof(Str),
    .print_fn = print_str,
    .hash_fn  = hash_str,
    .free_fn  = free_str
  },

  [EXP_LIST] = {
    .type     = EXP_LIST,
    .name     = "list",
    .obsize   = sizeof(List),
    .print_fn = print_list,
    .trace_fn = trace_list
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
    case NONE : t = EXP_NONE;        break;
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

hash_t hash_exp(Expr x) {
  hash_t out;
  ExpTypeInfo* info = exp_info(x);

  if ( info->hash_fn )
    out = info->hash_fn(x);

  else
    out = hash_word(x);

  return out;
}

void mark_exp(Expr x) {
  if ( exp_tag(x) == OBJ )
    mark_obj(as_obj(x));
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
void print_nul(FILE* ios, Expr x) {
  (void)x;
  fprintf(ios, "nul");
}

void print_none(FILE* ios, Expr x) {
  (void)x;
  fprintf(ios, "none");
}

// environment API
Env* mk_env(void) {
  Env* out = mk_obj(EXP_ENV);

  init_emap(&out->map);
  init_alist(&out->vals);

  return out;
}

Expr env_get(Env* e, Sym* n) {
  int i;
  Expr o = NONE;

  if ( emap_get(&e->map, n, &i) )
    o = (Expr)e->vals.vals[i];

  return o;
}

int  env_def(Env* e, Sym* n);
Expr env_set(Env* e, Sym* n, Expr x);

// symbol API
Sym* mk_sym(char* val) {
  Sym* s  = mk_obj(EXP_SYM); preserve(1, tag_obj(s));
  s->val  = mk_str(val);
  s->hash = hash_word(s->val->hash); // just munge the string hash

  return s;
}

bool sym_val_eql(Sym* s, char* v) {
  return strcmp(s->val->val, v) == 0;
}

void print_sym(FILE* ios, Expr x) {
  Sym* s = as_sym(x);

  fprintf(ios, "%s", s->val->val);
}

hash_t hash_sym(Expr x) {
  Sym* s = as_sym(x);

  return s->hash;
}

// string API
Strings StringTable = {
  .kvs       = NULL,
  .count     = 0,
  .max_count = 0
};

Str* new_str(char* cs, hash_t h, bool interned) {
  Str* s    = mk_obj(EXP_STR);

  s->val    = duplicates(cs);
  s->count  = strlen(cs);
  s->hash   = h;
  s->flags  = interned;

  return s;
}

void string_intern(Strings* t, StringsKV* kv, char* k, hash_t h) {
  (void)t;

  Str* s  = new_str(k, h, true);
  kv->val = s;
  kv->key = s->val;
}

Str* mk_str(char* cs) {
  size_t n = strlen(cs);
  Str* s;

  if ( n <= MAX_INTERN )
    s = strings_intern(&StringTable, cs, string_intern);

  else
    s = new_str(cs, hash_string(cs), false);

  return s;
}

void print_str(FILE* ios, Expr x) {
  Str* s = as_str(x);

  fprintf(ios, "\"%s\"", s->val);
}

hash_t hash_str(Expr x) {
  Str* s = as_str(x);

  return s->hash;
}

void free_str(void* ptr) {
  Str* s = ptr;

  if ( is_interned(s) ) // make sure to remove from Strings table before freeing
    strings_del(&StringTable, s->val, NULL);

  release(s->val, 0);
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

void trace_list(void* ptr) {
  List* xs = ptr;

  if ( xs->count ) {
    mark_exp(xs->head);
    mark_obj(xs->tail);
  }
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
