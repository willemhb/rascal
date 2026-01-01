#include <string.h>
#include <stdlib.h>

#include "util/util.h"
#include "val/val.h"
#include "val/sym.h"
#include "val/env.h"
#include "val/port.h"
#include "val/primitive.h"
#include "val/ffi.h"
#include "val/map.h"
#include "vm.h"
#include "lang.h"

// forward declarations
void print_nul(Port* ios, Expr x);
void print_type(Port* ios, Expr x);
void trace_type(RlState* rls, void* ptr);

bool bottom_has(Type* tx, Expr x);
bool top_has(Type* tx, Expr x);
bool datatype_has(Type* tx, Expr x);

// Static Type objects - these are permanent objects added to the permanent heap
Type TypeType = {
  .type     = &TypeType,
  .bfields  = FL_GRAY | FL_NOSWEEP,
  .tag      = EXP_TYPE,
  ._name    = "Type",
  .obsize   = sizeof(Type),
  .print_fn = print_type,
  .trace_fn = trace_type
};

Type NoneType = {
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_NONE,
  ._name    = "None",
  .has_fn   = bottom_has,
};

Type AnyType = {
  .type    = &TypeType,
  .bfields = FL_GRAY | FL_NOSWEEP,
  .tag     = EXP_ANY,
  ._name   = "Any",
  .has_fn  = top_has,
};

Type NulType = {
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_NUL,
  ._name    = "Nul",
  .obsize   = sizeof(Box),
  .print_fn = print_nul
};

// type APIs
bool bottom_has(Type* tx, Expr x) {
  (void)tx;
  (void)x;

  return false;
}

bool top_has(Type* tx, Expr x) {
  (void)tx;
  (void)x;

  return true;
}

bool datatype_has(Type* tx, Expr x) {
  return tx->tag == type_of(x)->tag;
}

bool has_type(Expr x, Type* t) {
  return t->has_fn(t, x);
}

Type* type_of(Expr x) {
  Type* t;

  switch ( tag_of(x) ) {
    case NUL_T   : t = &NulType;      break;
    case BOOL_T  : t = &BoolType;     break;
    case GLYPH_T : t = &GlyphType;    break;
    case BOX_T   : t = head(x)->type; break;
    case OBJ_T   : t = head(x)->type; break;
    default      : t = &NumType;      break;
  }

  return t;
}

char* type_name(Type* t) {
  return t->name->val->val;
}

// Initialize types - add static type objects to permanent heap
void init_builtin_type_hash(Type* type) {
  type->hashcode = hash_48(hash_word(hash_word(type->tag)));
}

void init_builtin_type(RlState* rls, Type* type) {
  type->name = mk_sym(rls, type->_name);
  type->has_fn = type->has_fn ? : datatype_has;
  add_to_permanent(rls, type);
  toplevel_env_def(rls, rls->vm->globals, type->name, tag_obj(type), false, true);
}

void register_builtin_types(RlState* rls) {
  /* most non-trivial hashes use the hash of the type as a seed value
     so that hashes of different types with identical underlying values,
     eg, (1 2 3) and [1 2 3] or 'symbol and "symbol", have distinct hashes.
     this means that the type hashcodes need to be initialized before anything
     else so that string and environments work correctly. */

  init_builtin_type_hash(&TypeType);
  init_builtin_type_hash(&NoneType);
  init_builtin_type_hash(&NulType);
  init_builtin_type_hash(&BoolType);
  init_builtin_type_hash(&GlyphType);
  init_builtin_type_hash(&ChunkType);
  init_builtin_type_hash(&RefType);
  init_builtin_type_hash(&UpValType);
  init_builtin_type_hash(&EnvType);
  init_builtin_type_hash(&CtlType);
  init_builtin_type_hash(&PortType);
  init_builtin_type_hash(&FunType);
  init_builtin_type_hash(&MethodType);
  init_builtin_type_hash(&MethodTableType);
  init_builtin_type_hash(&MTNodeType);
  init_builtin_type_hash(&SymType);
  init_builtin_type_hash(&StrType);
  init_builtin_type_hash(&ListType);
  init_builtin_type_hash(&TupleType);
  init_builtin_type_hash(&NumType);
  init_builtin_type_hash(&LibHandleType);
  init_builtin_type_hash(&ForeignFnType);
  init_builtin_type_hash(&MapType);
  init_builtin_type_hash(&MapNodeType);

  // initialize 
  init_builtin_type(rls, &TypeType);
  init_builtin_type(rls, &NoneType);
  init_builtin_type(rls, &NulType);
  init_builtin_type(rls, &BoolType);
  init_builtin_type(rls, &GlyphType);
  init_builtin_type(rls, &ChunkType);
  init_builtin_type(rls, &RefType);
  init_builtin_type(rls, &UpValType);
  init_builtin_type(rls, &EnvType);
  init_builtin_type(rls, &CtlType);
  init_builtin_type(rls, &PortType);
  init_builtin_type(rls, &FunType);
  init_builtin_type(rls, &MethodType);
  init_builtin_type(rls, &MethodTableType);
  init_builtin_type(rls, &MTNodeType);
  init_builtin_type(rls, &SymType);
  init_builtin_type(rls, &StrType);
  init_builtin_type(rls, &ListType);
  init_builtin_type(rls, &TupleType);
  init_builtin_type(rls, &NumType);
  init_builtin_type(rls, &LibHandleType);
  init_builtin_type(rls, &ForeignFnType);
  init_builtin_type(rls, &MapType);
  init_builtin_type(rls, &MapNodeType);
}

// utility array APIs
void trace_exprs(RlState* rls, Exprs* xs) {
  for ( int i=0; i < xs->count; i++ )
    mark_expr(rls, xs->data[i]);
}

void trace_objs(RlState* rls, Objs* os) {
  for ( int i=0; i < os->count; i++ )
    mark_obj(rls, os->data[i]);
}

// expression APIs
hash_t hash_expr(Expr x) {
  hash_t out;

  if ( !is_obj(x) )
    out = hash_word_48(x);

  else {
    Type* info = type_of(x);
    Obj* o = as_obj(x);

    if ( o->hashcode == 0 ) {
      if ( info->hash_fn )
        o->hashcode = info->hash_fn(x);

      else
        o->hashcode = hash_pointer_48(o);
    }

    out = o->hashcode;
  }

  assert(out != 0);

  return out;
}

bool egal_exprs(Expr x, Expr y) {
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

void mark_expr(RlState* rls, Expr x) {
  if ( expr_tag(x) == OBJ_T )
    mark_obj(rls, as_obj(x));
}

// object API
void* as_obj(Expr x) {
  return (void*)(x & XVMSK);
}

void* as_obj_s(RlState* rls, Type* t, Expr x) {
  require_argtype(rls, t, x);
  return as_obj(x);
}

Expr tag_obj(void* o) {
  return ((Expr)o) | OBJ_T;
}

void* mk_obj(RlState* rls, Type* type, flags_t flags) {
  Obj* out = allocate(rls, type->obsize);
  out->type = type;
  out->meta = NULL;
  out->bfields = flags | FL_GRAY;
  add_to_managed(rls, out);

  return out;
}

void* mk_obj_s(RlState* rls, Type* type, flags_t flags) {
  Obj* out = mk_obj(rls, type, flags);
  stack_push(rls, tag_obj(out));

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
  stack_push(rls, tag_obj(out));

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

// metadata
Expr get_meta(RlState* rls, Expr x, Expr k) {
  (void)rls;
  if ( !is_alloc(x) )
    return NONE;

  return map_get(metadata(x), k);
}

Expr set_meta(RlState* rls, Expr x, Expr k, Expr v) {
  if ( !is_alloc(x) )
    return NONE;

  metadata(x) = map_assoc(rls, metadata(x), k, v);
  return x;
}

Expr with_meta(RlState* rls, Expr x, Expr k, Expr v) {
  // like set_meta but allocates a box if necessary
  StackRef top = rls->s_top;

  if ( !is_alloc(x) ) {
    Box* out = box_expr_s(rls, x);
    x = tag_box(out);
  }

  metadata(x) = map_assoc(rls, metadata(x), k, v);
  rls->s_top = top;

  return x;
}

// helpers for dealing with metadata inside the VM
Expr vm_get_meta(RlState* rls, Expr x, char* key) {
  Sym* k = mk_sym_s(rls, key);
  Expr v = get_meta(rls, x, tag_obj(k));
  stack_pop(rls);
  return v;
}

Expr vm_set_meta(RlState* rls, Expr x, char* key, Expr v) {
  Sym* k = mk_sym_s(rls, key);
  Expr r = set_meta(rls, x, tag_obj(k), v);
  stack_pop(rls);
  return r;
}

Expr vm_with_meta(RlState* rls, Expr x, char* key, Expr v) {
  Sym* k = mk_sym_s(rls, key);
  x = with_meta(rls, x, tag_obj(k), v);
  stack_pop(rls);
  return x;
}

// print functions
void print_nul(Port* ios, Expr x) {
  (void)x;
  pprintf(ios, "nul");
}

void box_nul(Box* box, Expr init) {
  (void)init;

  box->pointer = NULL;
}

Expr unbox_nul(Box* box) {
  (void)box;

  return NUL;
}

bool init_nul(Box* box, void* spc) {
  (void)box;
  *(void**)spc = NULL;

  return true;
}

void print_type(Port* ios, Expr x) {
  Type* t = as_obj(x);
  pprintf(ios, "<type:%s>", type_name(t));
}

void trace_type(RlState* rls, void* ptr) {
  Type* t = ptr;
  mark_obj(rls, t->name);
}
