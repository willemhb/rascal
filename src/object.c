#include "util/number.h"
#include "util/hashing.h"

#include "vm.h"
#include "memory.h"
#include "equal.h"

#include "collection.h"
#include "type.h"
#include "object.h"

// external APIs
// flags
bool get_fl(void* p, flags_t f) {
  assert(p != NULL);
  Obj* o = p;

  if (f == 0)
    return o->flags;

  return !!(o->flags & f);
}

bool set_fl(void* p, flags_t f) {
  assert(p != NULL);
  Obj* o    = p;
  bool r    = !!(o->flags & f);
  o->flags |= f;

  return r;
}

bool del_fl(void* p, flags_t f) {
  assert(p != NULL);
  Obj* o    = p;
  bool r    = !!(o->flags & f);
  o->flags &= ~f;

  return r;
}

void* new_obj(Type* t, flags_t fl, size_t e) {
  void* o;
  size_t ns;
  
  ns = save(1, tag(t));
  o = allocate(&RlVm, t->v_table->obj_size + e);
  init_obj(o, t, fl);
  unsave(ns);

  return o;
}

void* clone_obj(void* p) {
  Obj* o;
  size_t ns;
  
  assert(p != NULL);
  ns = save(1, tag(p));
  o = duplicate(&RlVm, p, size_of(p));
  add_to_heap(o);
  unsave(ns);
  return o;
}

void init_obj(void* p, Type* t, flags_t fl) {
  Obj* o   = p;
  o->type  = t;
  o->annot = &EmptyMap;
  o->flags = fl;
  o->gray  = true;

  add_to_heap(o);
}

