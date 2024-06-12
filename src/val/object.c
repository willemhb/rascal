#include "val/object.h"
#include "val/type.h"

#include "vm/heap.h"

/* Common Object type and API functions */

// mark methods
void mark_val(Value x) {
  if ( is_obj(x) )
    mark_obj(as_obj(x));
}

void mark_obj(void* x) {
  if ( x ) {
    Object* o = x;

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
  Object* o = x;
  o->type->trace_fn(o);
  o->gray = false;
}

// other lifetime methods
void* new_obj(Type* t) {
  preserve(1, tag(t));
  Object* o = allocate(t->object_size, true);
  init_obj(t, o);

  return o;
}

void init_obj(Type* t, Object* o) {
  o->type  = t;
  o->meta  = NULL;
  o->hash  = 0;
  o->trace = true;
  o->free  = true;
  o->sweep = true;

  // Register in heap
  o->next           = Heap.live_objects;
  Heap.live_objects = o;
}

void free_obj(void* x) {
  Object* o = x;

  if ( o->free && o->type->destruct_fn )
    o->type->destruct_fn(o);
}

void sweep_obj(void* x) {
  Object* o = x;

  if ( o->sweep ) // allocated in heap
    deallocate(o, size_of(o, true), true);
}
