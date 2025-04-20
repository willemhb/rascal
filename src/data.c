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

bool   egal_syms(Expr x, Expr y);
bool   egal_strs(Expr x, Expr y);
bool   egal_lists(Expr x, Expr y);

void trace_fun(void* ptr);
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

  [EXP_FUN] = {
    .type     = EXP_FUN,
    .name     = "fun",
    .obsize   = sizeof(Fun),
    .trace_fn = trace_fun,
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

bool has_type(Expr x, ExpType t) {
  return exp_type(x) == t;
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

bool egal_exps(Expr x, Expr y) {
  bool out;
  
  if ( x != y )
    out = false;

  else {
    ExpType tx = exp_type(x), ty = exp_type(y);

    if ( tx != ty )
      out = false;

    else {
      EgalFn fn = Types[tx].egal_fn;
      out       = fn ? fn(x, y) : false;
    }
  }

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

void* mk_obj(ExpType type, flags_t flags) {
  Obj* out = allocate(true, Types[type].obsize);

  out->type     = type;
  out->bfields  = flags | FL_GRAY;
  out->heap     = Heap;
  Heap          = out;

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

// function API
Fun* mk_fun(Sym* name, OpCode op) {
  Fun* f   = mk_obj(EXP_FUN, 0);
  f->name  = name;
  f->label = op;

  return f;
}

void def_builtin_fun(char* name, OpCode op) {
  Sym* sym = mk_sym(name); preserve(1, tag_obj(sym));
  Fun* fun = mk_fun(sym, op);

  return def_builtin(&Globals, sym, tag_obj(fun));
}

void trace_fun(void* ptr) {
  Fun* fun = ptr;

  mark_obj(fun->name);
}

// environment API
void intern_in_env(EMap* m, EMapKV* kv, Sym* k, hash_t h) {
  (void)h;

  kv->key = k;
  kv->val = m->count-1;
}

Env* mk_env(void) {
  Env* out = mk_obj(EXP_ENV, 0);

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

int  env_def(Env* e, Sym* n) {
  int off = emap_intern(&e->map, n, intern_in_env);

  if ( off == e->map.count-1)
    alist_push(&e->vals, (void*)NONE);

  return off;
}

void env_set(Env* e, Sym* n, Expr x) {
  int off = env_def(e, n);

  e->vals.vals[off] = (void*)x;
}

void trace_env(void* ptr) {
  Env* e = ptr;

  if ( e->map.kvs )
    for ( int i=0, j=0; i < e->map.max_count && j < e->map.count; i++ ) {
      EMapKV* kv = &e->map.kvs[i];

      if ( kv->key != NULL ) {
        j++;
        mark_obj(kv->key);
      }
    }

  trace_exps(&e->vals);
}

void free_env(void* ptr) {
  Env* e = ptr;

  free_emap(&e->map);
  free_alist(&e->vals);
}

// symbol API
Sym* mk_sym(char* val) {
  Sym* s  = mk_obj(EXP_SYM, 0); preserve(1, tag_obj(s));
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

bool egal_syms(Expr x, Expr y) {
  Sym* sx = as_sym(x), * sy = as_sym(y);

  return sx->val == sy->val;
}

// string API
Strings StringTable = {
  .kvs       = NULL,
  .count     = 0,
  .max_count = 0
};

Str* new_str(char* cs, hash_t h, bool interned) {
  Str* s    = mk_obj(EXP_STR, 0);

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

bool egal_strs(Expr x, Expr y) {
  bool out;
  Str* sx = as_str(x), * sy = as_str(y);

  if ( sx->count != sy->count )
    out = false;

  else {
    if ( is_interned(sx) )
      out = sx == sy;

    else
      out = strcmp(sx->val, sy->val) == 0;
  }

  return out;
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

bool egal_lists(Expr x, Expr y) {
  
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
