#include "val/object.h"
#include "val/type.h"

#include "vm/state.h"
#include "vm/heap.h"

/* C Types */
/* Forward declarations */
// Helpers
static void* new_obj(State* vm, Type* t);
static void  init_obj(State* vm, Obj* o, Type* t, flags32 f);

// Interfaces
// External
/* Globals */
/* Helpers */
static void* new_obj(State* vm, Type* t) {
  Obj* o = rl_alloc(vm, vtbl(t)->osize);

  return o;
}

static void init_obj(State* vm, Obj* o, Type* t, flags32 f) {
  (void)vm;

  // initialize heajder fields
  o->heap   = NULL;
  o->meta   = NULL;
  o->hash   = 0;
  o->type   = t;
  o->vtype  = vtbl(t)->vtype;
  o->mflags = f | vtbl(t)->flags;
}


/* Interfaces */
/* APIs */
// cast methods
void* val_as_obj(Val x) {
  assert(is_obj(x));

  return (Obj*)(x & DATA_BITS);
}

void* ptr_as_obj(void* p) {
  assert(p != NULL);

  return p;
}

// mark methods
void val_mark(State* vm, Val x) {
  if ( is_obj(x) )
    obj_mark(vm, as_obj(x));
}

void obj_mark(State* vm, void* x) {
  Obj* o = x;
  
  if ( o ) {
    if ( o->black || o->notrace )
      return;

    o->black = true;

    // mark the metadata
    obj_mark(vm, o->meta);

    if ( vtbl(o)->trace )
      push_gray(vm, o);

    else
      o->gray = false;
  }
}

// trace method
void val_trace(State* vm, Val x) {
  assert(is_obj(x));

  obj_trace(vm, as_obj(x));
}

void obj_trace(State* vm, void* x) {
  TraceFn tf = vtbl(x)->trace;

  tf(vm, x);
}

// other lifetime methods
void* mk_obj(State* vm, Type* t, flags32 f) {
  Obj* o = new_obj(vm, t);

  init_obj(vm, o, t, f);
  add_to_heap(vm, o);

  return o;
}

void free_obj(State* vm, void* x) {
  FreeFn ff = vtbl(x)->free;

  if ( ff )
    ff(vm, x);
}

void sweep_obj(State* vm, void* x) {
  rl_dealloc(vm, x, vtbl(x)->osize);
}

void* clone_obj(State* vm, void* x, bool d) {
  assert(x != NULL);

  Obj* o = rl_dup(vm, x, vtbl(x)->osize);

  if ( d ) {
    CloneFn cf = vtbl(x)->clone;

    if ( cf )
      cf(vm, o, d);
  }

  return o;
}

void* seal_obj(State* vm, void* x, bool d) {
  assert(x != NULL);

  Obj* o = x;

  if ( !o->sealed ) {
    o->sealed = true;

    SealFn sf = vtbl(o)->seal;

    if ( sf )
      sf(vm, o, d);
  }

  return o;
}

void* unseal_obj(State* vm, void* x) {
  assert(x != NULL);

  Obj* o = x;

  if ( o->sealed ) {
    o         = clone_obj(vm, o, true);
    o->sealed = false;
    o->hash   = 0;                       // invalidate existing hash
  }

  return o;
}
