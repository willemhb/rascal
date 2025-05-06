#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "data.h"
#include "runtime.h"
#include "lang.h"
#include "collection.h"
#include "util.h"

// forward declarations
void print_ref(Port* ios, Expr x);
void print_list(Port* ios, Expr x);
void print_sym(Port* ios, Expr x);
void print_str(Port* ios, Expr x);
void print_num(Port* ios, Expr x);
void print_bool(Port* ios, Expr x);
void print_glyph(Port* ios, Expr x);
void print_nul(Port* ios, Expr x);
void print_none(Port* ios, Expr x);

hash_t hash_sym(Expr x);
hash_t hash_str(Expr x);

bool egal_syms(Expr x, Expr y);
bool egal_strs(Expr x, Expr y);
bool egal_lists(Expr x, Expr y);

void trace_chunk(void* ptr);
void trace_alist(void* ptr);
void trace_ref(void* ptr);
void trace_upval(void* ptr);
void trace_env(void* ptr);
void trace_fun(void* ptr);
void trace_list(void* ptr);

void free_alist(void* ptr);
void free_buf16(void* ptr);
void free_env(void* ptr);
void free_port(void* ptr);
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

  [EXP_GLYPH] = {
    .type     = EXP_GLYPH,
    .name     = "glyph",
    .obsize   = 0,
    .print_fn = print_glyph
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

  [EXP_BUF16] = {
    .type     = EXP_BUF16,
    .name     = "buf16",
    .obsize   = sizeof(Buf16),
    .free_fn  = free_buf16
  },

  [EXP_REF] = {
    .type     = EXP_REF,
    .name     = "ref",
    .obsize   = sizeof(Ref),
    .print_fn = print_ref,
    .trace_fn = trace_ref
  },

  [EXP_UPV] = {
    .type     = EXP_UPV,
    .name     = "upval",
    .obsize   = sizeof(UpVal),
    .trace_fn = trace_upval
  },

  [EXP_ENV] = {
    .type     = EXP_ENV,
    .name     = "env",
    .obsize   = sizeof(Env),
    .trace_fn = trace_env,
    .free_fn  = free_env
  },

  [EXP_PORT] = {
    .type     = EXP_PORT,
    .name     = "port",
    .obsize   = sizeof(Port),
    .free_fn  = free_port
  },

  [EXP_FUN] = {
    .type     = EXP_FUN,
    .name     = "fun",
    .obsize   = sizeof(Fun),
    .trace_fn = trace_fun,
  },

  [EXP_SYM] = {
    .type     = EXP_SYM,
    .name     = "sym",
    .obsize   = sizeof(Sym),
    .print_fn = print_sym,
    .hash_fn  = hash_sym,
    .egal_fn  = egal_syms
  },

  [EXP_STR] = {
    .type     = EXP_STR,
    .name     = "str",
    .obsize   = sizeof(Str),
    .print_fn = print_str,
    .hash_fn  = hash_str,
    .egal_fn  = egal_strs,
    .free_fn  = free_str
  },

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
ExpType exp_type(Expr x) {
  ExpType t;

  switch ( x & XTMSK ) {
    case NONE_T  : t = EXP_NONE;      break;
    case NUL_T   : t = EXP_NUL;       break;
    case EOS_T   : t = EXP_EOS;       break;
    case BOOL_T  : t = EXP_BOOL;      break;
    case GLYPH_T : t = EXP_GLYPH;     break;
    case OBJ_T   : t = head(x)->type; break;
    default      : t = EXP_NUM;       break;
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
void print_nul(Port* ios, Expr x) {
  (void)x;
  pprintf(ios, "nul");
}

void print_none(Port* ios, Expr x) {
  (void)x;
  pprintf(ios, "none");
}

// chunk API
Chunk* mk_chunk(Env* vars, Alist* vals, Buf16* code) {
  Chunk* out = mk_obj(EXP_CHUNK, 0);

  out->vars = vars;
  out->vals = vals;
  out->code = code;

  return out;
}

void dis_chunk(Chunk* chunk) {
  instr_t* instr = chunk->code->binary.vals;
  int offset     = 0, max_offset = chunk->code->binary.count;

  printf("%-8s %-16s %-5s %-5s\n\n", "line", "instruction", "input", "input");

  while ( offset < max_offset ) {
    OpCode op  = instr[offset];
    int argc   = op_arity(op);
    char* name = op_name(op);

    switch ( argc ) {
   
      case 1: { 
        instr_t arg = instr[offset+1];
        printf("%.8d %-16s %.5d -----\n", offset, name, arg);
        offset += 2;                                           // advance past argument
        break;
      }

      case -2: { // variadic
        int arg = instr[offset+1];
        printf("%.8d %-16s %.5d -----\n", offset, name, arg);
        offset++;

        for ( int i=0; i < arg; i++, offset += 2 ) {
          int x = instr[offset+1], y = instr[offset+2];
          printf("%.8d ---------------- %.5d %.5d\n", offset, x, y);
        }

        break;
      }

      default:
        printf("%.8d %-16s ----- -----\n", offset, name);
        offset++;
        break;
    }
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
  Alist* out = mk_obj(EXP_ALIST, 0); init_exprs(&out->exprs);

  return out;
}

int alist_push(Alist* a, Expr x) {
  exprs_push(&a->exprs, x);

  return a->exprs.count;
}

Expr alist_pop(Alist* a) {
  return exprs_pop(&a->exprs);
}

Expr alist_get(Alist* a, int n) {
  assert(n >= 0 && n < a->exprs.count);

  return a->exprs.vals[n];
}

void trace_alist(void* ptr) {
  Alist* a = ptr;

  trace_exprs(&a->exprs);
}

void free_alist(void* ptr) {
  Alist* a = ptr;
  
  free_exprs(&a->exprs);
}

// buf16 API
Buf16* mk_buf16(void) {
  Buf16* b = mk_obj(EXP_BUF16, 0); init_bin16(&b->binary);

  return b;
}

int buf16_write(Buf16* b, ushort_t *xs, int n) {
  bin16_write(&b->binary, xs, n);

  return b->binary.count;
}

void free_buf16(void* ptr) {
  Buf16* b = ptr;

  free_bin16(&b->binary);
}

// reference API
Ref* mk_ref(Sym* n, int o) {
  Ref* ref  = mk_obj(EXP_REF, 0);
  ref->name = n;
  ref->ref_type = REF_UNDEF; // filled in by env_put, env_resolve, &c
  ref->offset = o;

  return ref;
}

void print_ref(Port* ios, Expr x) {
  Ref* r = as_ref(x);

  pprintf(ios, "#'%s", r->name->val->val);
}

void trace_ref(void* ptr) {
  Ref* r = ptr;

  mark_obj(r->name);
}

// upval API
UpVal* mk_upval(UpVal* next, Expr* loc) {
  // only open upvalues can be created
  UpVal* upv  = mk_obj(EXP_UPV, 0);
  upv->next   = next;
  upv->closed = false;
  upv->loc    = loc;

  return upv;
}

Expr* deref(UpVal* upv) {
  return upv->closed ? &upv->val : upv->loc;
}

void trace_upval(void* ptr) {
  UpVal* upv = ptr;

  if ( upv->closed )
    mark_exp(upv->val);
}

// environment API
void intern_in_env(EMap* m, EMapKV* kv, Sym* k, hash_t h) {
  (void)h;

  kv->key = k;
  kv->val = mk_ref(k, m->count-1);
}

Env* mk_env(Env* parent) {
  Env* env = mk_obj(EXP_ENV, 0);

  env->parent = parent;
  env->arity  = 0;
  env->ncap   = 0;
  init_emap(&env->vars);
  init_emap(&env->upvs);

  return env;
}

Ref* env_capture(Env* e, Ref* r) {
  assert(is_local_env(e));

  Ref* c = emap_intern(&e->upvs, r->name, intern_in_env);

  if ( c->ref_type == REF_UNDEF ) {
    c->ref_type = r->ref_type == REF_LOCAL ? REF_LOCAL_UPVAL : REF_CAPTURED_UPVAL;
    c->captures = r;
  }

  return c;
}

Ref* env_resolve(Env* e, Sym* n, bool capture) {
  Ref* r = NULL;

  if ( is_global_env(e) )
    emap_get(&e->vars, n, &r);

  else {
    bool found;

    found = emap_get(&e->vars, n, &r); // check locals first

    if ( found ) {
      if ( capture ) {         // resolved from enclosed context
        r = env_capture(e, r);
        e->ncap++;             // so we know to emit the capture instruction
      }

    } else {
      // check already captured upvalues
      found = emap_get(&e->upvs, n, &r);

      if ( !found ) {
        r = env_resolve(e->parent, n, true);

        if ( r != NULL && r->ref_type != REF_GLOBAL )
          r = env_capture(e, r);
      }
    }
  }

  return r;
}

Ref* env_define(Env* e, Sym* n) {
  Ref* ref = emap_intern(&e->vars, n, intern_in_env);

  if ( ref->ref_type == REF_UNDEF ) {
    if ( is_local_env(e) )
      ref->ref_type = REF_LOCAL;

    else {
      ref->ref_type = REF_GLOBAL;
      exprs_push(&e->vals, NONE); // reserve space for value
    }
  }

  return ref;
}

// helpers for working with the global environment
void toplevel_env_def(Env* e, Sym* n, Expr x) {
  assert(is_global_env(e));
  assert(!is_keyword(n));

  Ref* r = env_define(e, n);

  e->vals.vals[r->offset] = x;
}

void toplevel_env_set(Env* e, Sym* n, Expr x) {
  assert(is_global_env(e));
  assert(!is_keyword(n));

  Ref* r = toplevel_env_find(e, n);

  assert(r != NULL);

  e->vals.vals[r->offset] = x;
}

void toplevel_env_refset(Env* e, int n, Expr x) {
  assert(is_global_env(e));
  assert(n >= 0 && n < e->vals.count);
  e->vals.vals[n] = x;
}

Ref* toplevel_env_find(Env* e, Sym* n) {
  assert(is_global_env(e));

  Ref* ref = NULL;

  emap_get(&e->vars, n, &ref);

  return ref;
}

Expr toplevel_env_ref(Env* e, int n) {
  assert(is_global_env(e));
  assert(n >= 0 && n < e->vals.count);

  return e->vals.vals[n];
}

Expr toplevel_env_get(Env* e, Sym* n) {
  assert(is_global_env(e));
  Expr x = NONE;
  Ref* ref = toplevel_env_find(e, n);

  if ( ref !=  NULL )
    x = e->vals.vals[ref->offset];

  return x;
}

static void trace_emap(EMap* m) {
  for ( int i=0, j=0; i < m->max_count && j < m->count; i++ ) {
    EMapKV* kv = &m->kvs[i];

    if ( kv->key != NULL ) {
      j++;
      mark_obj(kv->key);
      mark_obj(kv->val);
    }
  }
}

void trace_env(void* ptr) {
  Env* e = ptr;

  mark_obj(e->parent);

  trace_emap(&e->vars);

  if ( is_local_env(e) )
    trace_emap(&e->upvs);

  else
    trace_exprs(&e->vals);
}

void free_env(void* ptr) {
  Env* e = ptr;

  free_emap(&e->vars);

  if ( is_local_env(e) )
    free_emap(&e->upvs);

  else
    free_exprs(&e->vals);
}

// port API -------------------------------------------------------------------
Port* mk_port(FILE* ios) {
  Port* p = mk_obj(EXP_PORT, 0);
  p->ios  = ios;

  return p;
}

Port* open_port(char* fname, char* mode) {
  FILE* ios = fopen(fname, mode);

  require(ios != NULL, "couldn't open %s: %s", fname, strerror(errno));

  return mk_port(ios);
}

void  close_port(Port* port) {
  if ( port->ios != NULL ) {
    fclose(port->ios);
    port->ios = NULL;
  }
}

// stdio.h wrappers -----------------------------------------------------------
// at some point we hope to create a better
// port implementation using lower-level
// C functions, so it makes sense to create
// these APIs now
bool peof(Port* p) {
  // don't call on a closed port
  assert(p->ios != NULL);

  return feof(p->ios);
}

int pseek(Port* p, long off, int orig) {
  // don't call on a closed port
  assert(p->ios != NULL);

  return fseek(p->ios, off, orig);
}

Glyph pgetc(Port* p) {
  // don't call on a closed port
  assert(p->ios != NULL);

  return fgetc(p->ios);
}

Glyph pungetc(Port* p, Glyph g) {
  // don't call on a closed port
  assert(p->ios != NULL);

  return ungetc(g, p->ios);
}

Glyph ppeekc(Port* p) {
  // don't call on a closed port
  assert(p->ios != NULL);

  int c = fgetc(p->ios);

  if ( c != EOF )
    ungetc(c, p->ios);

  return c;
}

int pprintf(Port* p, char* fmt, ...) {
  // don't call on a closed port
  assert(p->ios != NULL);
  va_list va;
  va_start(va, fmt);
  int o = vfprintf(p->ios, fmt, va);
  va_end(va);
  return o;
}

int pvprintf(Port* p, char* fmt, va_list va) {
  // don't call on a closed port
  assert(p->ios != NULL);

  int o = vfprintf(p->ios, fmt, va);
  return o;
}

// lifetime methods -----------------------------------------------------------
void free_port(void* ptr) {
  Port* p = ptr;

  close_port(p);
}

// function API
Fun* as_fun_s(char* f, Expr x) {
  ExpType t = exp_type(x);
  require(t == EXP_FUN, "%s wanted a fun, got %s", f, Types[t].name);

  return as_fun(x);
}

Fun* mk_fun(Sym* name, OpCode op, Chunk* code) {
  Fun* f   = mk_obj(EXP_FUN, 0);
  f->name  = name;
  f->label = op;
  f->chunk = code;

  init_objs(&f->upvs);

  return f;
}

Fun* mk_closure(Fun* proto) {
  Fun* cls; int count = user_fn_upvalc(proto);

  if ( count == 0 )
    cls = proto;

  else {
    cls = clone_obj(proto);
    objs_write(&cls->upvs, NULL, count);
  }

  return cls;
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

  toplevel_env_def(&Globals, sym, tag_obj(fun));
}

void disassemble(Fun* fun) {
  printf("\n\n==== %s ====\n\n", fun->name->val->val);
  dis_chunk(fun->chunk);
  printf("\n\n");
}

Expr upval_ref(Fun* fun, int i) {
  assert(i >= 0 && i < fun->upvs.count);
  UpVal* upv = fun->upvs.vals[i];

  return *deref(upv);
}

void upval_set(Fun* fun, int i, Expr x) {
  assert(i >= 0 && i < fun->upvs.count);
  UpVal* upv = fun->upvs.vals[i];

  *deref(upv) = x;
}

void trace_fun(void* ptr) {
  Fun* fun = ptr;

  mark_obj(fun->name);
  mark_obj(fun->chunk);
}

// symbol API
Sym* as_sym_s(char* f, Expr x) {
  ExpType t = exp_type(x);
  require(t == EXP_SYM, "%s wanted a sym, got %s", f, Types[t].name);

  return as_sym(x);
}

Sym* mk_sym(char* val) {
  Sym* s  = mk_obj(EXP_SYM, 0); preserve(1, tag_obj(s));
  s->val  = mk_str(val);
  s->hash = hash_word(s->val->hash); // just munge the string hash

  return s;
}

bool sym_val_eql(Sym* s, char* v) {
  return streq(s->val->val, v);
}

void print_sym(Port* ios, Expr x) {
  Sym* s = as_sym(x);

  pprintf(ios, "%s", s->val->val);
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

Str* as_str_s(char* f, Expr x) {
  ExpType t = exp_type(x);
  require(t == EXP_STR, "%s wanted a str, got %s", f, Types[t].name);

  return as_str(x);
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

void print_str(Port* ios, Expr x) {
  Str* s = as_str(x);

  pprintf(ios, "\"%s\"", s->val);
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

// boolean APIs
Bool as_bool(Expr x) {
  return x == TRUE;
}

Expr tag_bool(Bool b) {
  return b ? TRUE : FALSE;
}

void print_bool(Port* ios, Expr x) {
  pprintf(ios, x == TRUE ? "true" : "false");
}

// glyph APIs
Glyph as_glyph(Expr x) {
  return x & XVMSK;
}

Expr tag_glyph(Glyph x) {
  return ((Expr)x) | GLYPH_T;
}

void print_glyph(Port* ios, Expr x) {
  Glyph g = as_glyph(x);

  if ( g < CHAR_MAX && CharNames[g] )
    pprintf(ios, "\\%s", CharNames[g]);

  else
    pprintf(ios, "\\%c", g);
}
