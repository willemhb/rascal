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
void print_method(Port* ios, Expr x);
void print_fun(Port* ios, Expr x);
void print_mtable(Port* ios, Expr x);
void print_nul(Port* ios, Expr x);
void print_none(Port* ios, Expr x);
void print_type(Port* ios, Expr x);

hash_t hash_sym(Expr x);
hash_t hash_str(Expr x);

bool egal_syms(Expr x, Expr y);
bool egal_strs(Expr x, Expr y);
bool egal_lists(Expr x, Expr y);

void clone_method(RlState* rls, void* ptr);

void trace_chunk(RlState* rls, void* ptr);
void trace_alist(RlState* rls, void* ptr);
void trace_ref(RlState* rls, void* ptr);
void trace_upval(RlState* rls, void* ptr);
void trace_env(RlState* rls, void* ptr);
void trace_method(RlState* rls, void* ptr);
void trace_fun(RlState* rls, void* ptr);
void trace_mtable(RlState* rls, void* ptr);
void trace_sym(RlState* rls, void* ptr);
void trace_list(RlState* rls, void* ptr);
void trace_type(RlState* rls, void* ptr);

void free_alist(RlState* rls, void* ptr);
void free_buf16(RlState* rls, void* ptr);
void free_env(RlState* rls, void* ptr);
void free_port(RlState* rls, void* ptr);
void free_str(RlState* rls, void* ptr);

bool bottom_has(Type* tx, Type* ty);
bool datatype_has(Type* tx, Type* ty);

// Static Type objects - these are permanent objects added to the permanent heap
Type TypeType = {
  .heap     = NULL,
  .type     = &TypeType,  // self-referential
  .bfields  = FL_GRAY,
  .tag      = EXP_TYPE,
  .obsize   = sizeof(Type),
  .print_fn = print_type,
  .trace_fn = trace_type
};

Type NoneType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_NONE,
  .obsize   = 0,
  .print_fn = print_none
};

Type NulType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_NUL,
  .obsize   = 0,
  .print_fn = print_nul
};

Type EosType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_EOS,
  .obsize   = 0
};

Type BoolType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_BOOL,
  .obsize   = 0,
  .print_fn = print_bool
};

Type GlyphType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_GLYPH,
  .obsize   = 0,
  .print_fn = print_glyph
};

Type ChunkType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_CHUNK,
  .obsize   = sizeof(Chunk),
  .trace_fn = trace_chunk
};

Type AlistType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_ALIST,
  .obsize   = sizeof(Alist),
  .trace_fn = trace_alist,
  .free_fn  = free_alist
};

Type Buf16Type = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_BUF16,
  .obsize   = sizeof(Buf16),
  .free_fn  = free_buf16
};

Type RefType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_REF,
  .obsize   = sizeof(Ref),
  .print_fn = print_ref,
  .trace_fn = trace_ref
};

Type UpValType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_UPV,
  .obsize   = sizeof(UpVal),
  .trace_fn = trace_upval
};

Type EnvType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_ENV,
  .obsize   = sizeof(Env),
  .trace_fn = trace_env,
  .free_fn  = free_env
};

Type PortType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_PORT,
  .obsize   = sizeof(Port),
  .free_fn  = free_port
};

Type MethodType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_METHOD,
  .obsize   = sizeof(Method),
  .clone_fn = clone_method,
  .trace_fn = trace_method,
  .print_fn = print_method
};

Type MethodTableType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_MTABLE,
  .obsize   = sizeof(MethodTable),
  .trace_fn = trace_mtable,
  .print_fn = print_mtable
};

Type FunType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_FUN,
  .obsize   = sizeof(Fun),
  .trace_fn = trace_fun,
  .print_fn = print_fun
};

Type SymType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_SYM,
  .obsize   = sizeof(Sym),
  .trace_fn = trace_sym,
  .print_fn = print_sym,
  .hash_fn  = hash_sym,
  .egal_fn  = egal_syms
};

Type StrType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_STR,
  .obsize   = sizeof(Str),
  .print_fn = print_str,
  .hash_fn  = hash_str,
  .egal_fn  = egal_strs,
  .free_fn  = free_str
};

Type ListType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_LIST,
  .obsize   = sizeof(List),
  .print_fn = print_list,
  .egal_fn  = egal_lists,
  .trace_fn = trace_list
};

Type NumType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_NUM,
  .obsize   = 0,
  .print_fn = print_num
};

// Type lookup array (indexed by ExpType)
// Initialize types - add static type objects to permanent heap
void init_builtin_type(RlState* rls, Type* type, char* name) {
  type->name = mk_sym(rls, name);
  type->has_fn = type->tag == EXP_NONE ? bottom_has : datatype_has;
  add_to_permanent(rls, type);
  toplevel_env_def(rls, rls->vm->globals, type->name, tag_obj(type));
}

void register_builtin_types(RlState* rls) {
  init_builtin_type(rls, &TypeType, "Type");
  init_builtin_type(rls, &NoneType, "None");
  init_builtin_type(rls, &NulType, "Nul");
  init_builtin_type(rls, &EosType, "Eos");
  init_builtin_type(rls, &BoolType, "Bool");
  init_builtin_type(rls, &GlyphType, "Glyph");
  init_builtin_type(rls, &ChunkType, "Chunk");
  init_builtin_type(rls, &AlistType, "Alist");
  init_builtin_type(rls, &Buf16Type, "Buf-16");
  init_builtin_type(rls, &RefType, "Ref");
  init_builtin_type(rls, &UpValType, "UpVal");
  init_builtin_type(rls, &EnvType, "Env");
  init_builtin_type(rls, &PortType, "Port");
  init_builtin_type(rls, &MethodType, "Method");
  init_builtin_type(rls, &FunType, "Fun");
  init_builtin_type(rls, &MethodTableType, "MethodTable");
  init_builtin_type(rls, &SymType, "Sym");
  init_builtin_type(rls, &StrType, "Str");
  init_builtin_type(rls, &ListType, "List");
  init_builtin_type(rls, &NumType, "Num");
}

// utility array APIs
void trace_exprs(RlState* rls, Exprs* xs) {
  for ( int i=0; i < xs->count; i++ )
    mark_exp(rls, xs->vals[i]);
}

void trace_objs(RlState* rls, Objs* os) {
  for ( int i=0; i < os->count; i++ )
    mark_obj(rls, os->vals[i]);
}

// type APIs
bool bottom_has(Type* tx, Type* ty) {
  (void)tx;
  (void)ty;

  return false;
}

bool datatype_has(Type* tx, Type* ty) {
  return tx->tag == ty->tag;
}

ExpType exp_type(Expr x) {
  ExpType t;

  switch ( x & XTMSK ) {
    case NONE_T  : t = EXP_NONE;           break;
    case NUL_T   : t = EXP_NUL;            break;
    case EOS_T   : t = EXP_EOS;            break;
    case BOOL_T  : t = EXP_BOOL;           break;
    case GLYPH_T : t = EXP_GLYPH;          break;
    case OBJ_T   : t = head(x)->type->tag; break;
    default      : t = EXP_NUM;            break;
  }

  return t;
}

bool has_type(Expr x, Type* t) {
  Type* xt = type_of(x);

  return t->has_fn(t, xt);
}

Type* type_of(Expr x) {
  Type* t;
  
  switch ( x & XTMSK ) {
    case NONE_T  : t = &NoneType;     break;
    case NUL_T   : t = &NulType;      break;
    case EOS_T   : t = &EosType;      break;
    case BOOL_T  : t = &BoolType;     break;
    case GLYPH_T : t = &GlyphType;    break;
    case OBJ_T   : t = head(x)->type; break;
    default      : t = &NumType;      break;
  }

  return t;
}

char* type_name(Type* t) {
  return t->name->val->val;
}

hash_t hash_exp(Expr x) {
  hash_t out;
  Type* info = type_of(x);

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
   Type* tx = type_of(x), * ty = type_of(y);

    if ( tx->tag != ty->tag )
      out = false;

    else {
      EgalFn fn = tx->egal_fn;
      out       = fn ? fn(x, y) : false;
    }
  }

  return out;
}

void mark_exp(RlState* rls, Expr x) {
  if ( exp_tag(x) == OBJ_T )
    mark_obj(rls, as_obj(x));
}

// object API
void* as_obj(Expr x) {
  return (void*)(x & XVMSK);
}

Expr tag_obj(void* o) {
  return ((Expr)o) | OBJ_T;
}

void* mk_obj(RlState* rls, Type* type, flags_t flags) {
  Obj* out = allocate(rls, type->obsize);
  out->type = type;
  out->bfields = flags | FL_GRAY;
  add_to_managed(rls, out);

  return out;
}

void* mk_obj_s(RlState* rls, Type* type, flags_t flags) {
  Obj* out = mk_obj(rls, type, flags);
  push(rls, tag_obj(out));

  return out;
}

void* clone_obj(RlState* rls, void* ptr) {
  assert(ptr != NULL);

  Obj* obj = ptr;
  Type* info = obj->type;
  Obj* out = duplicate(rls, info->obsize, obj);

  if ( info->clone_fn )
    info->clone_fn(rls, out);

  return out;
}

void* clone_obj_s(RlState* rls, void* ptr) {
  void* out = clone_obj(rls, ptr);
  push(rls, tag_obj(out));

  return out;
}

void mark_obj(RlState* rls, void* ptr) {
  Obj* obj = ptr;

  if ( obj != NULL ) {
    if ( obj->black == false ) {
      obj->black = true;

      Type* info = obj->type;

      if ( info->trace_fn )
        gc_save(rls, obj);

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

void free_obj(RlState* rls, void* ptr) {
  Obj* obj = ptr;

  if ( obj ) {
    Type* info = obj->type;

    if ( info->free_fn )
      info->free_fn(rls, obj);

    release(rls, obj, info->obsize);
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

void print_type(Port* ios, Expr x) {
  Type* t = as_obj(x);
  pprintf(ios, "<type:%s>", type_name(t));
}

void trace_type(RlState* rls, void* ptr) {
  Type* t = ptr;
  mark_obj(rls, t->name);
}

// chunk API
Chunk* mk_chunk(RlState* rls, Env* vars, Alist* vals, Buf16* code) {
  Chunk* out = mk_obj(rls, &ChunkType, 0);

  out->vars = vars;
  out->vals = vals;
  out->code = code;

  return out;
}

Chunk* mk_chunk_s(RlState* rls, Env* vars, Alist* vals, Buf16* code) {
  Chunk* out = mk_chunk(rls, vars, vals, code);
  push(rls, tag_obj(out));

  return out;
}

void dis_chunk(Chunk* chunk) {
  instr_t* instr = chunk->code->binary.vals;
  int offset     = 0, max_offset = chunk->code->binary.count;

  printf("%-8s %-16s %-5s %-5s %-8s\n\n", "line", "instruction", "input", "input", "constant");

  while ( offset < max_offset ) {
    OpCode op  = instr[offset];
    int argc   = op_arity(op);
    char* name = op_name(op);

    switch ( argc ) {
   
      case 1: { 
        instr_t arg = instr[offset+1];
        printf("%.8d %-16s %.5d ----- ", offset, name, arg);

        if ( op == OP_GET_VALUE ) {
          print_exp(&Outs, chunk->vals->exprs.vals[arg]);

        } else if ( op == OP_GLYPH ) {
          if ( arg < CHAR_MAX && CharNames[arg])
            printf("\\%s", CharNames[arg]);

          else
            printf("\\%c", arg);
        } else if ( op == OP_SMALL ) {
          printf("%d", (short)arg);

        } else
          printf("--------");

        printf("\n");
        offset += 2; // advance past argument
        break;
      }

      case -2: { // variadic
        int arg = instr[offset+1];
        printf("%.8d %-16s %.5d ----- --------\n", offset, name, arg);
        offset++;

        for ( int i=0; i < arg; i++, offset += 2 ) {
          int x = instr[offset+1], y = instr[offset+2];
          printf("%.8d ---------------- %.5d %.5d\n", offset, x, y);
        }

        break;
      }

      default:
        printf("%.8d %-16s ----- ----- ", offset, name);

        if ( op == OP_TRUE )
          printf("%-8s", "true");

        else if ( op == OP_FALSE )
          printf("%-8s", "false");

        else if ( op == OP_NUL )
          printf("%-8s", "nul");

        else if ( op == OP_ZERO )
          printf("%-8s", "0");

        else if ( op == OP_ONE )
          printf("%-8s", "1");

        else
          printf("--------");

        printf("\n");
        
        offset++;
        break;
    }
  }
}

void trace_chunk(RlState* rls, void* ptr) {
  Chunk* chunk = ptr;

  mark_obj(rls, chunk->vars);
  mark_obj(rls, chunk->vals);
  mark_obj(rls, chunk->code);
}

// alist API
Alist* mk_alist(RlState* rls) {
  Alist* out = mk_obj(rls, &AlistType, 0);
  init_exprs(rls, &out->exprs);

  return out;
}

Alist* mk_alist_s(RlState* rls) {
  Alist* out = mk_alist(rls);
  push(rls, tag_obj(out));

  return out;
}

int alist_push(RlState* rls, Alist* a, Expr x) {
  exprs_push(rls, &a->exprs, x);

  return a->exprs.count;
}

Expr alist_pop(RlState* rls, Alist* a) {
  return exprs_pop(rls, &a->exprs);
}

Expr alist_get(Alist* a, int n) {
  assert(n >= 0 && n < a->exprs.count);

  return a->exprs.vals[n];
}

void trace_alist(RlState* rls, void* ptr) {
  Alist* a = ptr;

  trace_exprs(rls, &a->exprs);
}

void free_alist(RlState* rls, void* ptr) {
  Alist* a = ptr;

  free_exprs(rls, &a->exprs);
}

// buf16 API
Buf16* mk_buf16(RlState* rls) {
  Buf16* b = mk_obj(rls, &Buf16Type, 0);
  init_bin16(rls, &b->binary);
  return b;
}

Buf16* mk_buf16_s(RlState* rls) {
  Buf16* out = mk_buf16(rls);
  push(rls, tag_obj(out));

  return out;
}

int buf16_write(RlState* rls, Buf16* b, ushort_t *xs, int n) {
  bin16_write(rls, &b->binary, xs, n);

  return b->binary.count;
}

void free_buf16(RlState* rls, void* ptr) {
  Buf16* b = ptr;

  free_bin16(rls, &b->binary);
}

// reference API
Ref* mk_ref(RlState* rls, Sym* n, int o) {
  Ref* ref  = mk_obj(rls, &RefType, 0);
  ref->name = n;
  ref->ref_type = REF_UNDEF; // filled in by env_put, env_resolve, &c
  ref->offset = o;
  ref->val = NONE;

  return ref;
}

void print_ref(Port* ios, Expr x) {
  Ref* r = as_ref(x);

  pprintf(ios, "#'%s", r->name->val->val);
}

void trace_ref(RlState* rls, void* ptr) {
  Ref* r = ptr;

  mark_obj(rls, r->captures);
  mark_obj(rls, r->name);
  mark_exp(rls, r->val);
}

// upval API
UpVal* mk_upval(RlState* rls, UpVal* next, Expr* loc) {
  // only open upvalues can be created
  UpVal* upv  = mk_obj(rls, &UpValType, 0);
  upv->next   = next;
  upv->closed = false;
  upv->loc    = loc;

  return upv;
}

Expr* deref(UpVal* upv) {
  return upv->closed ? &upv->val : upv->loc;
}

void trace_upval(RlState* rls, void* ptr) {
  UpVal* upv = ptr;

  if ( upv->closed )
    mark_exp(rls, upv->val);
}

// environment API
void intern_in_env(RlState* rls, EMap* m, EMapKV* kv, Sym* k, hash_t h) {
  (void)h;

  kv->key = k;
  kv->val = mk_ref(rls, k, m->count-1);
}

Env* mk_env(RlState* rls, Env* parent) {
  Env* env = mk_obj(rls, &EnvType, 0);

  env->parent = parent;
  env->arity = 0;
  env->ncap = 0;
  env->va = false;
  env->etype = parent == NULL ? REF_GLOBAL : REF_LOCAL;
  init_emap(rls, &env->vars);
  init_emap(rls, &env->upvs);

  return env;
}

Env* mk_env_s(RlState* rls, Env* parent) {
  Env* out = mk_env(rls, parent);
  push(rls, tag_obj(out));

  return out;
}

Ref* env_capture(RlState* rls, Env* e, Ref* r) {
  assert(is_local_env(e));

  Ref* c = emap_intern(rls, &e->upvs, r->name, intern_in_env);

  if ( c->ref_type == REF_UNDEF ) {
    c->ref_type = r->ref_type == REF_LOCAL ? REF_LOCAL_UPVAL : REF_CAPTURED_UPVAL;
    c->captures = r;
  }

  return c;
}

Ref* env_resolve(RlState* rls, Env* e, Sym* n, bool capture) {
  Ref* r = NULL;

  if ( is_global_env(e) )
    emap_get(rls, &e->vars, n, &r);

  else {
    bool found;

    found = emap_get(rls, &e->vars, n, &r); // check locals first

    if ( found ) {
      if ( capture ) {         // resolved from enclosed context
        r = env_capture(rls, e, r);
        e->ncap++;             // so we know to emit the capture instruction
      }

    } else {
      // check already captured upvalues
      found = emap_get(rls, &e->upvs, n, &r);

      if ( !found ) {
        r = env_resolve(rls, e->parent, n, true);

        if ( r != NULL && r->ref_type != REF_GLOBAL )
          r = env_capture(rls, e, r);
      }
    }
  }

  return r;
}

Ref* env_define(RlState* rls, Env* e, Sym* n) {
  Ref* ref = emap_intern(&Main, &e->vars, n, intern_in_env);

  if ( ref->ref_type == REF_UNDEF ) {
    if ( is_local_env(e) )
      ref->ref_type = REF_LOCAL;

    else {
      ref->ref_type = REF_GLOBAL;
      objs_push(rls, &e->vals, ref); // reserve space for value
    }
  }

  return ref;
}

// helpers for working with the global environment
void toplevel_env_def(RlState* rls, Env* e, Sym* n, Expr x) {
  assert(is_global_env(e));
  assert(!is_keyword(n));

  Ref* r = env_define(rls, e, n);
  r->val = x;

  if ( exp_type(x) == EXP_FUN ) // fill in function name for better debugging
    as_fun(x)->name = r->name;
}

void toplevel_env_set(RlState* rls, Env* e, Sym* n, Expr x) {
  assert(is_global_env(e));
  assert(!is_keyword(n));

  Ref* r = toplevel_env_find(rls, e, n);
  assert(r != NULL);

  // handle function-to-function assignment: add method to existing Fun
  if (is_fun(x)) {
    Fun* new_fun = as_fun(x);

    if (is_fun(r->val)) {
      // add the new method to the existing function
      Fun* existing = as_fun(r->val);
      Method* new_method = new_fun->singleton;
      assert(new_method != NULL);  // new Fun always has a singleton initially
      fun_add_method(rls, existing, new_method);
      new_fun->name = r->name;
      return;  // don't overwrite r->val
    }
  }

  // non-function assignment to function binding: error
  if (is_fun(r->val)) {
    eval_error(rls, "cannot assign non-function to %s", n->val->val);
  }

  r->val = x;

  if (is_fun(x))
    as_fun(x)->name = r->name;
}

void toplevel_env_refset(RlState* rls, Env* e, int n, Expr x) {
  (void)rls;

  assert(is_global_env(e));
  assert(n >= 0 && n < e->vals.count);
  Ref* r = e->vals.vals[n];
  r->val = x;

  if ( is_fun(x) ) // fill in function name for better debugging
    as_fun(x)->name = r->name;
}

Ref* toplevel_env_find(RlState* rls, Env* e, Sym* n) {
  assert(is_global_env(e));

  Ref* ref = NULL;

  emap_get(rls, &e->vars, n, &ref);

  return ref;
}

Expr toplevel_env_ref(RlState* rls, Env* e, int n) {
  (void)rls;

  assert(is_global_env(e));
  assert(n >= 0 && n < e->vals.count);

  Ref* r = e->vals.vals[n];

  return r->val;
}

Expr toplevel_env_get(RlState* rls, Env* e, Sym* n) {
  assert(is_global_env(e));
  Expr x = NONE;
  Ref* ref = toplevel_env_find(rls, e, n);

  if ( ref !=  NULL )
    x = ref->val;

  return x;
}

static void trace_emap(RlState* rls, EMap* m) {
  for ( int i=0, j=0; i < m->max_count && j < m->count; i++ ) {
    EMapKV* kv = &m->kvs[i];

    if ( kv->key != NULL ) {
      j++;
      mark_obj(rls, kv->key);
      mark_obj(rls, kv->val);
    }
  }
}

void trace_env(RlState* rls, void* ptr) {
  Env* e = ptr;

  mark_obj(rls, e->parent);

  trace_emap(rls, &e->vars);

  if ( is_local_env(e) )
    trace_emap(rls, &e->upvs);

  else
    trace_objs(rls, &e->vals);
}

void free_env(RlState* rls, void* ptr) {
  Env* e = ptr;

  free_emap(rls, &e->vars);

  if ( is_local_env(e) )
    free_emap(rls, &e->upvs);

  else
    free_objs(rls, &e->vals);
}

// port API -------------------------------------------------------------------
Port* mk_port(RlState* rls, FILE* ios) {
  Port* p = mk_obj(rls, &PortType, 0);
  p->ios  = ios;

  return p;
}

Port* mk_port_s(RlState* rls, FILE* ios) {
  Port* out = mk_port(rls, ios);
  push(rls, tag_obj(out));

  return out;
}

Port* open_port(RlState* rls, char* fname, char* mode) {
  FILE* ios = fopen(fname, mode);

  require(rls, ios != NULL, "couldn't open %s: %s", fname, strerror(errno));

  return mk_port(rls, ios);
}

Port* open_port_s(RlState* rls, char* fname, char* mode) {
  Port* out = open_port(rls, fname, mode);
  push(rls, tag_obj(out));

  return out;
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
void free_port(RlState* rls, void* ptr) {
  (void)rls;
  Port* p = ptr;

  close_port(p);
}

// method API
Method* mk_method(RlState* rls, Sym* name, int arity, bool va, OpCode op, Chunk* code) {
  Method* m = mk_obj(rls, &MethodType, 0);
  m->name = name;
  m->arity = arity;
  m->va = va;
  m->label = op;
  m->chunk = code;

  assert(code != NULL || op != OP_NOOP);

  init_objs(rls, &m->upvs);

  return m;
}

Method* mk_method_s(RlState* rls, Sym* name, int arity, bool va, OpCode op, Chunk* code) {
  Method* out = mk_method(rls, name, arity, va, op, code);
  push(rls, tag_obj(out));
  return out;
}

Method* mk_closure(RlState* rls, Method* proto) {
  Method* cls; int count = method_upvalc(proto);

  if ( count == 0 )
    cls = proto;

  else {
    cls = clone_obj(rls, proto);
    objs_write(rls, &cls->upvs, NULL, count);
  }

  return cls;
}

Method* mk_builtin_method(RlState* rls, Sym* name, int arity, bool va, OpCode op) {
  return mk_method(rls, name, arity, va, op, NULL);
}

Method* mk_user_method(RlState* rls, Chunk* code) {
  int sp = save_sp(rls);
  Sym* n = mk_sym_s(rls, "method");
  int arity = code->vars->arity;
  bool va = code->vars->va;
  Method* m = mk_method(rls, n, arity, va, OP_NOOP, code);
  restore_sp(rls, sp);

  return m;
}

Method* mk_user_method_s(RlState* rls, Chunk* code) {
  Method* out = mk_user_method(rls, code);
  push(rls, tag_obj(out));
  return out;
}

void disassemble(Method* m) {
  printf("\n\n==== %s ====\n\n", m->name->val->val);
  dis_chunk(m->chunk);
  printf("\n\n");
}

Expr upval_ref(Method* m, int i) {
  assert(i >= 0 && i < m->upvs.count);
  UpVal* upv = m->upvs.vals[i];

  return *deref(upv);
}

void upval_set(Method* m, int i, Expr x) {
  assert(i >= 0 && i < m->upvs.count);
  UpVal* upv = m->upvs.vals[i];

  *deref(upv) = x;
}

void print_method(Port* ios, Expr x) {
  Method* m = as_method(x);

  pprintf(ios, "<method:%s/%d%s>",
          m->name->val->val,
          m->arity,
          m->va ? "+" : "");
}

void clone_method(RlState* rls, void* ptr) {
  Method* m = ptr;

  if ( m->upvs.vals != NULL )
    m->upvs.vals = duplicate(rls, m->upvs.max_count, m->upvs.vals);
}

void trace_method(RlState* rls, void* ptr) {
  Method* m = ptr;

  mark_obj(rls, m->name);
  mark_obj(rls, m->chunk);
  trace_objs(rls, &m->upvs);
}

// method table API
MethodTable* mk_mtable(RlState* rls) {
  MethodTable* mt = mk_obj(rls, &MethodTableType, 0);
  mt->variadic = NULL;
  init_objs(rls, &mt->methods);
  return mt;
}

MethodTable* mk_mtable_s(RlState* rls) {
  MethodTable* out = mk_mtable(rls);
  push(rls, tag_obj(out));
  return out;
}

void mtable_add(RlState* rls, MethodTable* mt, Method* m) {
  // check for duplicate signature
  for (int i = 0; i < mt->methods.count; i++) {
    Method* existing = mt->methods.vals[i];
    if (method_argc(existing) == method_argc(m) && method_va(existing) == method_va(m)) {
      eval_error(rls, "method with arity %d%s already exists",
                 method_argc(m), method_va(m) ? "+" : "");
    }
  }
  objs_push(rls, &mt->methods, m);
}

Method* mtable_lookup(MethodTable* mt, int argc) {
  for (int i = 0; i < mt->methods.count; i++) {
    Method* m = mt->methods.vals[i];
    if (method_argc(m) == argc && !method_va(m))
      return m;  // exact match
  }

  if ( mt->variadic != NULL && method_argc(mt->variadic) <= argc )
    return mt->variadic;

  return NULL; // failure
}

void print_mtable(Port* ios, Expr x) {
  (void)x;
  pprintf(ios, "<method-table>");
}

void trace_mtable(RlState* rls, void* ptr) {
  MethodTable* mt = ptr;
  trace_objs(rls, &mt->methods);
}

// function API
Fun* as_fun_s(RlState* rls, char* f, Expr x) {
  require_argtype(rls, f, &FunType, x);

  return as_fun(x);
}

Fun* mk_fun(RlState* rls, Sym* name, Method* m) {
  Fun* f = mk_obj(rls, &FunType, 0);
  f->name = name;
  f->num_methods = 1;
  f->singleton = m;
  f->methods = NULL;

  return f;
}

Fun* mk_fun_s(RlState* rls, Sym* name, Method* m) {
  Fun* out = mk_fun(rls, name, m);
  push(rls, tag_obj(out));
  return out;
}

void fun_add_method(RlState* rls, Fun* fun, Method* m) {
  if (fun->singleton != NULL && fun->methods == NULL) {
    // convert from singleton to method table
    Method* old = fun->singleton;
    fun->methods = mk_mtable_s(rls);
    mtable_add(rls, fun->methods, old);
    fun->singleton = NULL;
  }

  if (fun->methods != NULL) {
    mtable_add(rls, fun->methods, m);
  } else {
    // should not happen, but handle it
    fun->singleton = m;
  }

  fun->num_methods++;
}

Method* fun_lookup(Fun* fun, int argc) {
  if (fun->singleton != NULL) {
    Method* m = fun->singleton;
    if (method_argc(m) == argc && !method_va(m))
      return m;
    if (method_va(m) && argc >= method_argc(m))
      return m;
    return NULL;
  }

  if (fun->methods != NULL) {
    return mtable_lookup(fun->methods, argc);
  }

  return NULL;
}

void def_builtin_fun(RlState* rls, char* name, int arity, bool va, OpCode op) {
  int sp = save_sp(rls);
  Sym* sym = mk_sym_s(rls, name);
  Method* m = mk_builtin_method(rls, sym, arity, va, op);
  Fun* fun = mk_fun(rls, sym, m);
  toplevel_env_def(rls, Vm.globals, sym, tag_obj(fun));
  restore_sp(rls, sp);
}

Fun* mk_user_fun(RlState* rls, Chunk* code) {
  Method* m = mk_user_method(rls, code);
  return mk_fun(rls, m->name, m);
}

Fun* mk_user_fun_s(RlState* rls, Chunk* code) {
  Fun* out = mk_user_fun(rls, code);
  push(rls, tag_obj(out));
  return out;
}

void print_fun(Port* ios, Expr x) {
  Fun* fun = as_fun(x);

  pprintf(ios, "<fun:%s/%d>", fun->name->val->val, fun->num_methods);
}

void trace_fun(RlState* rls, void* ptr) {
  Fun* fun = ptr;

  mark_obj(rls, fun->name);
  mark_obj(rls, fun->singleton);
  mark_obj(rls, fun->methods);
}

// symbol API
Sym* as_sym_s(RlState* rls, char* f, Expr x) {
  require_argtype(rls, f, &SymType, x);
  return as_sym(x);
}

Sym* mk_sym(RlState* rls, char* val) {
  int sp = save_sp(rls);
  Sym* s = mk_obj_s(rls, &SymType, 0);
  s->val  = mk_str(rls, val);
  s->hash = hash_word(s->val->hash); // just munge the string hash
  restore_sp(rls, sp);

  return s;
}

Sym* mk_sym_s(RlState* rls, char* val) {
  Sym* out = mk_sym(rls, val);
  push(rls, tag_obj(out));
  return out;
}

bool sym_val_eql(Sym* s, char* v) {
  return streq(s->val->val, v);
}

void trace_sym(RlState* rls, void* ptr) {
  Sym* s = ptr;

  mark_obj(rls, s->val);
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

Str* new_str(RlState* rls, char* cs, hash_t h, bool interned) {
  Str* s = mk_obj(rls, &StrType, 0);
  s->val = duplicates(rls, cs);
  s->count = strlen(cs);
  s->hash = h;
  s->flags = interned;

  return s;
}

void string_intern(RlState* rls, Strings* t, StringsKV* kv, char* k, hash_t h) {
  (void)t;

  Str* s  = new_str(rls, k, h, true);
  kv->val = s;
  kv->key = s->val;
}

Str* as_str_s(RlState* rls, char* f, Expr x) {
  require_argtype(rls, f, &StrType, x);
  return as_str(x);
}

Str* mk_str(RlState* rls, char* cs) {
  size_t n = strlen(cs);
  Str* s;

  if ( n <= MAX_INTERN )
    s = strings_intern(rls, rls->vm->strings, cs, string_intern);

  else
    s = new_str(rls, cs, hash_string(cs), false);

  return s;
}

Str* mk_str_s(RlState* rls, char* cs) {
  Str* out = mk_str(rls, cs);
  push(rls, tag_obj(out));
  return out;
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

void free_str(RlState* rls, void* ptr) {
  Str* s = ptr;

  if ( is_interned(s) ) // make sure to remove from Strings table before freeing
    strings_del(rls, &StringTable, s->val, NULL);

  release(rls, s->val, 0);
}

// list API
List* as_list_s(RlState* rls, char* f, Expr x) {
  require_argtype(rls, f, &ListType, x);

  return as_list(x);
}

List* empty_list(RlState* rls) {
  List* l = mk_obj(rls, &ListType, 0);

  l->head  = NUL;
  l->tail  = NULL;
  l->count = 0;

  return l;
}

List* empty_list_s(RlState* rls) {
  List* out = empty_list(rls);
  push(rls, tag_obj(out));
  return out;
}

List* mk_list(RlState* rls, size_t n, Expr* xs) {
  int sp = save_sp(rls);
  List* l = empty_list(rls);

  if ( n > 0 ) {
    push(rls, tag_obj(l));

    for ( size_t i=n; i>0; i-- ) {
      l = cons(rls, xs[i-1], l);
      tos(rls) = tag_obj(l);
    }
  }

  restore_sp(rls, sp);
  return l;
}

List* mk_list_s(RlState* rls, size_t n, Expr* xs) {
  List* out = mk_list(rls, n, xs);
  push(rls, tag_obj(out));
  return out;
}

List* cons(RlState* rls, Expr hd, List* tl) {
  assert(tl != NULL);
  int sp = save_sp(rls);
  push(rls, tag_obj(tl));
  List* l = mk_obj(rls, &ListType, 0);
  l->head = hd;
  l->tail = tl;
  l->count = tl->count+1;
  restore_sp(rls, sp);
  return l;
}

List* cons_s(RlState* rls, Expr hd, List* tl) {
  List* out = cons(rls, hd, tl);
  push(rls, tag_obj(out));
  return out;
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

void trace_list(RlState* rls, void* ptr) {
  List* xs = ptr;

  if ( xs->count ) {
    mark_exp(rls, xs->head);
    mark_obj(rls, xs->tail);
  }
}

// number APIs
Num as_num_s(RlState* rls, char* f, Expr x) {
  require_argtype(rls, f, &NumType, x);
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

Glyph as_glyph_s(RlState* rls, char* f, Expr x) {
  require_argtype(rls, f, &GlyphType, x);
  return as_glyph(x);
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
