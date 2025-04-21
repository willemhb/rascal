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
void print_bool(FILE* ios, Expr x);
void print_nul(FILE* ios, Expr x);
void print_none(FILE* ios, Expr x);

hash_t hash_sym(Expr x);
hash_t hash_str(Expr x);

bool   egal_syms(Expr x, Expr y);
bool   egal_strs(Expr x, Expr y);
bool   egal_lists(Expr x, Expr y);

void trace_chunk(void* ptr);
void trace_alist(void* ptr);
void trace_fun(void* ptr);
void trace_env(void* ptr);
void trace_list(void* ptr);

void free_alist(void* ptr);
void free_buffer(void* ptr);
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

  [EXP_BOOL] = {
    .type     = EXP_BOOL,
    .name     = "bool",
    .obsize   = 0,
    .print_fn = print_bool
  },

  [EXP_CHUNK] = {
    .type     = EXP_CHUNK,
    .name     = "chunk",
    .obsize   = sizeof(Chunk),
    .trace_fn = trace_chunk
  },

  [EXP_ALIST] = {
    .type     = EXP_ALIST,
    .name     = "alist",
    .obsize   = sizeof(Alist),
    .trace_fn = trace_alist,
    .free_fn  = free_alist
  },

  [EXP_BUFFER] = {
    .type     = EXP_BUFFER,
    .name     = "buffer",
    .obsize   = sizeof(Buffer),
    .free_fn  = free_buffer
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
    case NONE_T : t = EXP_NONE;      break;
    case NUL_T  : t = EXP_NUL;       break;
    case EOS_T  : t = EXP_EOS;       break;
    case BOOL_T : t = EXP_BOOL;      break;
    case OBJ_T  : t = head(x)->type; break;
    default     : t = EXP_NUM;       break;
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
  
  if ( x == y )
    out = true;

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
  if ( exp_tag(x) == OBJ_T )
    mark_obj(as_obj(x));
}

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

// chunk API
Chunk* mk_chunk(Env* vars, Alist* vals, Buffer* code) {
  Chunk* out = mk_obj(EXP_CHUNK, 0);

  out->vars = vars;
  out->vals = vals;
  out->code = code;

  return out;
}

void dis_chunk(Chunk* chunk) {
  instr_t* instr = (instr_t*)chunk->code->binary.vals;
  int offset     = 0, max_offset = chunk->code->binary.count / 2;

  printf("%-8s %-16s %-5s\n\n", "line", "instruction", "input");

  while ( offset < max_offset ) {
    OpCode op  = instr[offset];
    int argc   = op_arity(op);
    char* name = op_name(op);

    if ( argc == 1 ) {
      instr_t arg = instr[offset+1];
      printf("%.8d %-16s %.5d\n", offset, name, arg);
      offset++;                                     // advance past argument
    } else
      printf("%.8d %-16s -----\n", offset, name);

    offset++;
  }
}

void trace_chunk(void* ptr) {
  Chunk* chunk = ptr;

  mark_obj(chunk->vars);
  mark_obj(chunk->vals);
  mark_obj(chunk->code);
}

// alist API
Alist* mk_alist(void) {
  Alist* out = mk_obj(EXP_ALIST, 0); init_stack(&out->stack);

  return out;
}

int alist_push(Alist* a, Expr x) {
  stack_push(&a->stack, (void*)x);

  return a->stack.count;
}

Expr alist_pop(Alist* a) {
  return (Expr)stack_pop(&a->stack);
}

Expr alist_get(Alist* a, int n) {
  assert(n >= 0 && n < a->stack.count);

  return (Expr)a->stack.vals[n];
}

void trace_alist(void* ptr) {
  Alist* a = ptr;

  trace_exps(&a->stack);
}

void free_alist(void* ptr) {
  Alist* a = ptr;
  
  free_stack(&a->stack);
}

// buffer API
Buffer* mk_buffer(void) {
  Buffer* b = mk_obj(EXP_BUFFER, 0); init_binary(&b->binary);

  return b;
}

int buffer_write(Buffer* b, byte_t c) {
  binary_write(&b->binary, c);

  return b->binary.count;
}

int buffer_write_n(Buffer* b, byte_t *cs, int n) {
  binary_write_n(&b->binary, cs, n);

  return b->binary.count;
}

void free_buffer(void* ptr) {
  Buffer* b = ptr;

  free_binary(&b->binary);
}

// function API
Fun* as_fun_s(Expr x) {
  ExpType t = exp_type(x);
  require(t == EXP_FUN, "expected type fun, got %s", Types[t].name);

  return as_fun(x);
}

Fun* mk_fun(Sym* name, OpCode op, Chunk* code) {
  Fun* f   = mk_obj(EXP_FUN, 0);
  f->name  = name;
  f->label = op;
  f->chunk = code;
  
  return f;
}

Fun* mk_builtin_fun(Sym* name, OpCode op) {
  return mk_fun(name, op, NULL);
}

Fun* mk_user_fun(Chunk* code) {
  Sym* n = mk_sym("fn"); preserve(1, tag_obj(n));
  Fun* f = mk_fun(n, OP_NOOP, code);

  return f;
}

void def_builtin_fun(char* name, OpCode op) {
  Sym* sym = mk_sym(name); preserve(1, tag_obj(sym));
  Fun* fun = mk_builtin_fun(sym, op);

  env_set(&Globals, sym, tag_obj(fun));
}

void disassemble(Fun* fun) {
  printf("\n\n==== %s ====\n\n", fun->name->val->val);
  dis_chunk(fun->chunk);
  printf("\n\n");
}

void trace_fun(void* ptr) {
  Fun* fun = ptr;

  mark_obj(fun->name);
  mark_obj(fun->chunk);
}

// environment API
void intern_in_env(EMap* m, EMapKV* kv, Sym* k, hash_t h) {
  (void)h;

  kv->key = k;
  kv->val = m->count-1;
}

Env* mk_env(bool local) {
  Env* out = mk_obj(EXP_ENV, 0);
  out->local = local;
  out->arity = 0;
  init_emap(&out->map);
  init_stack(&out->vals);

  return out;
}

Expr env_get(Env* e, Sym* n) {
  int i;
  Expr o = NONE;

  if ( emap_get(&e->map, n, &i) )
    o = (Expr)e->vals.vals[i];

  return o;
}

Expr env_ref(Env* e, int n) {
  assert(!e->local);

  Expr o = NONE;

  if ( n >= 0 && n < e->vals.count )
    o = (Expr)e->vals.vals[n];

  return o;
}

int  env_put(Env* e, Sym* n) {
  int off = emap_intern(&e->map, n, intern_in_env);

  if ( !e->local && off == e->map.count-1 )
    stack_push(&e->vals, (void*)NONE);

  return off;
}

void env_set(Env* e, Sym* n, Expr x) {
  assert(!e->local);
  int off = env_put(e, n);

  e->vals.vals[off] = (void*)x;
}

void env_refset(Env* e, int n, Expr x) {
  assert(!e->local);
  assert(n >= 0 && n < e->vals.count);

  e->vals.vals[n] = (void*)x;
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
Num as_num_s(Expr x) {
  ExpType t = exp_type(x);
  require(t == EXP_NUM, "expected type num, got %s", Types[t].name);
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

void print_num(FILE* ios, Expr x) {
  fprintf(ios, "%g", as_num(x));
}

// boolean APIs
Bool as_bool(Expr x) {
  return x == TRUE;
}

Expr tag_bool(Bool b) {
  return b ? TRUE : FALSE;
}

void print_bool(FILE* ios, Expr x) {
  fprintf(ios, x == TRUE ? "true" : "false");
}
