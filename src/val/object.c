#include "val/object.h"

#include "vm/state.h"
#include "vm/heap.h"

/* Internal APIs */
/* External APIs */
// mark methods
void mark_val(State* vm, Val x) {
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

    if ( tracefn(o) )
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
  TraceFn tf = tracefn(x);

  tf(vm, x);
}

// other lifetime methods
void* new_obj(State* vm, Type t, flags32 f) {
  Obj* o = rl_alloc(vm, obsize(t));

  init_obj(vm, o, t, f);

  return o;
}

void init_obj(State* vm, Obj* o, Type t, flags32 f) {

  // initialize heajder fields
  o->meta   = NULL;
  o->hash   = 0;
  o->tag    = t;
  o->mflags = f | MF_GRAY;

  // register object in heap
  o->heap   = vm->heap;
  vm->heap  = o;
}

void free_obj(State* vm, void* x) {
  FreeFn ff = freefn(x);

  if ( ff )
    ff(vm, x);
}

void sweep_obj(State* vm, void* x) {
  rl_dealloc(vm, x, obsize(x));
}
