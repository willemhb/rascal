#include "val/object.h"
#include "val/type.h"

#include "vm/heap.h"

/* Common Object type and API functions */

// mark methods
rl_status_t mark_val(Value x) {
  rl_status_t out = OKAY;
  
  if ( is_obj(x) )
    out = mark_obj(as_obj(x));

  return out;
}

rl_status_t mark_obj(void* x) {
  rl_status_t out = OKAY;
  
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

  return out;
}

// trace method
rl_status_t trace(void* x) {
  rl_status_t out = OKAY;
  Object* o       = x;

  o->type->trace_fn(o);

  o->gray = false;

  return out;
}

// other lifetime methods
rl_status_t new_obj(Type* t, void** r) {
  preserve(1, tag(t));

  rl_status_t out = allocate(r, t->object_size, true);

  if ( out == OKAY )
    out = init_obj(t, *r);

  return out;
}

rl_status_t init_obj(Type* t, Object* o) {
  o->type  = t;
  o->meta  = NULL;
  o->hash  = 0;
  o->trace = true;
  o->free  = true;
  o->sweep = true;

  // Register in heap
  o->next           = Heap.live_objects;
  Heap.live_objects = o;

  return OKAY;
}

rl_status_t free_obj(void* x) {
  rl_status_t out = OKAY;
  Object*     o   = x;

  if ( o->free && o->type->destruct_fn )
    out = o->type->destruct_fn(o);

  return out;
}

rl_status_t sweep_obj(void* x) {
  rl_status_t out = OKAY;
  Object*     o   = x;

  if ( o->sweep ) // allocated in heap
    out = deallocate(o, size_of(o, true), true);

  return out;
}
