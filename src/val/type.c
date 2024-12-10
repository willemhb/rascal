#include "val/type.h"
#include "val/box.h"
#include "val/func.h"
#include "val/env.h"

#include "vm/state.h"
#include "vm/heap.h"

#include "util/hash.h"

/* C types */

/* Forward declarations */
// Helpers
static VType little_vt(Val x);

// Interfaces
void   trace_type(State* vm, void* x);
hash64 hash_type(Val x);
bool   egal_types(Val x, Val y);
int    order_types(Val x, Val y);

// External

/* Globals */
VTable TypeVTable = {
  .vtype = T_TYPE,
  .flags = MF_SEALED,
  .dsize = sizeof(Type*),
  .osize = sizeof(Type),
  .tag   = OBJECT,
  .trace = trace_type,
  .hash  = hash_type,
  .egal  = egal_types,
  .order = order_types
};

Func TypeCtor = {
  .type    = &FuncType,
  .vtype   = T_FUNC,
  ._name   = "Type"
};

Type TypeType = {
  .type    = &TypeType,
  .vtype   = T_TYPE,
  ._name   = "Type",
  .id      = T_TYPE,
  .kind    = K_VALUE,
  .vtable  = &TypeVTable,
};

Type AnyType = {};

Type NoneType = {};

/* Helpers */
static VType little_vt(Val x) {
  return x >> WTAG_SHIFT && WTYPE_MASK;
}

/* Interfaces */

/* API */
// get the Type or VType
Type* type_of_val(Val x) {
  switch ( tag_bits(x) ) {
    default:       return &NumType;
    case CPOINTER: return &PtrType;
    case LITTLE:   return Vm.types[little_vt(x)];
    case OBJECT:   return obtype(x);
  }
}

Type* type_of_obj(void* x) {
  assert(x != NULL);

  return ((Obj*)x)->type;
}

VType vtype_of_val(Val x) {
  switch ( tag_bits(x) ) {
    default:       return T_NUM;
    case CPOINTER: return T_PTR;
    case LITTLE:   return little_vt(x);
    case OBJECT:   return obtype(x)->vtype;
  }
}

VType vtype_of_obj(void* x) {
  assert(x != NULL);

  return ((Obj*)x)->vtype;
}

// check the type or VType
bool val_has_type(Val x, Type* t) {
  bool r;
  
  switch ( t->kind ) {
    case K_BOTTOM: r = false;                   break;
    case K_VALUE:  r = type_of(x)->id == t->id; break;
    case K_TOP:    r = true;                    break;
  }

  return r;
}

bool obj_has_type(void* x, Type* t) {
  assert(x != NULL);

  bool r;
  Obj* o = x;

  switch ( t->kind ) {
    case K_BOTTOM: r = false;                break;
    case K_VALUE:  r = o->type->id == t->id; break;
    case K_TOP:    r = true;                 break;
  }

  return r;
}

bool val_has_vtype(Val x, VType t) {
  return vtype_of(x) == t;
}

bool obj_has_vtype(void* x, VType t) {
  return vtype_of(x) == t;
}

// vtbl methods
VTable* val_vtbl(Val x) {
  Type* t = type_of(x);

  return t->vtable;
}

VTable* type_vtbl(Type* t) {
  assert(t != NULL);

  return t->vtable;
}

VTable* vtype_vtbl(VType t) {
  return Vm.types[t]->vtable;
}

VTable* obj_vtbl(void* x) {
  Type* t = type_of(x);

  return t->vtable;
}

/* Initialization */
void init_builtin_type(State* vm, Type* t) {
  static hash64 type_hash = 0;

  if ( type_hash == 0 )
    type_hash = hash_word(T_TYPE);

  t->hash    = mix_hashes(type_hash, hash_word(t->id));
  t->sealed  = true;
  t->nosweep = true;

  if ( t->id < T_NONE )
    vm->types[t->id] = t;

  add_to_heap(vm, t);

  if ( t->ctor ) { // handle initialization of ctor
    t->ctor->nosweep = true;

    add_to_heap(vm, t->ctor);
  }
}

void define_builtin_type(State* vm, Type* t) {
  /*
   * define builtin type in global environment.
   * 
   * this is done after every builtin type has been initialized
   * to ensure that hashing works correctly.
   */

  Ref* r = def_global(vm->globals, t->name);

  init_ref(r, tag(t));

  t->name       = r->name;
  t->ctor->name = r->name;
}
