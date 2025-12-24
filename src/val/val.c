#include <string.h>
#include <stdlib.h>

#include "util/util.h"
#include "val/val.h"
#include "val/sym.h"
#include "val/env.h"
#include "val/port.h"
#include "vm.h"
#include "lang.h"

// forward declarations
void print_nul(Port* ios, Expr x);
void print_none(Port* ios, Expr x);
void print_type(Port* ios, Expr x);
void trace_type(RlState* rls, void* ptr);

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

// type APIs
bool bottom_has(Type* tx, Type* ty) {
  (void)tx;
  (void)ty;

  return false;
}

bool datatype_has(Type* tx, Type* ty) {
  return tx->tag == ty->tag;
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

// Initialize types - add static type objects to permanent heap
void init_builtin_type(RlState* rls, Type* type, char* name) {
  type->name = mk_sym(rls, name);
  type->has_fn = type->tag == EXP_NONE ? bottom_has : datatype_has;
  add_to_permanent(rls, type);
  toplevel_env_def(rls, rls->vm->globals, type->name, tag_obj(type), false, true);
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
  init_builtin_type(rls, &FunType, "Fun");
  init_builtin_type(rls, &MethodType, "Method");
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

// expression APIs
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
      out = fn ? fn(x, y) : false;
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

void* as_obj_s(RlState* rls, char* fn, Type* t, Expr x) {
  if ( !has_type(x, t) ) {
    fprintf(stderr, "eval error in %s: wanted a %s, got ", fn, type_name(t));
    print_exp(&Errs, x);
    fprintf(stderr, ".\n");
    rl_longjmp(rls, EVAL_ERROR);
  }

  return as_obj(x);
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

// print functions
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
