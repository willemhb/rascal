#include "val/object.h"
#include "val/type.h"

#include "vm/heap.h"

/* Common Obj type and API functions */

// mark methods
void mark_val(Val x) {
  if ( is_obj(x) )
    mark_obj(as_obj(x));
}

void mark_obj(void* x) {
  if ( x ) {
    Obj* o = x;

    if ( !o->black ) {
      o->black = true;

      if ( o->trace && o->type->trace_fn )
        push_gray(o);

      else
        o->gray = false;
    }
  }
}

// trace method
void trace(void* x) {
  Obj* o = x;
  o->type->trace_fn(o);
  o->gray = false;
}

// other lifetime methods
void* new_obj(Type* t) {
  preserve(1, tag(t));
  Obj* o = allocate(t->obj_size, true);
  init_obj(t, o);

  return o;
}

void init_obj(Type* t, Obj* o) {
  o->type  = t;
  o->meta  = NULL;
  o->hash  = 0;
  o->trace = true;
  o->free  = true;
  o->sweep = true;

  // Register in heap
  o->next   = Heap.objs;
  Heap.objs = o;
}

void free_obj(void* x) {
  Obj* o = x;

  if ( o->free && o->type->free_fn )
    o->type->free_fn(o);
}

void sweep_obj(void* x) {
  Obj* o = x;

  if ( o->sweep ) // allocated in heap
    deallocate(o, size_of(o, true), true);
}
