#include "val/object.h"
#include "val/environ.h"
#include "val/table.h"
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

void* clone_obj(void* x) {
  assert(x);

  Obj* o  = x;
  Type* t = o->type;
  void* r = duplicate(o, size_of(o, true), true);

  if ( t->clone_fn )
    t->clone_fn(r);

  return r;
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

// metadata methods
void intern_metadata(void* t, void* e, void* k, void* s, hash_t h) {
  (void)t;
  (void)s;
  (void)h;

  MMEntry* me = e;
  Map* m = mk_map(0, NULL);

  me->key = (Val)k;
  me->val = tag(m);
}

Map* val_meta(Val x) {
  Map* r;
  
  if ( is_obj(x) )
    r = obj_meta(as_obj(x));

  else {
    MMEntry* me;

    mmap_intern(&MetaData, x, &me, intern_metadata, NULL);

    r = as_map(me->val);
  }

  return r;
}

Map* obj_meta(void* x) {
  assert(x);

  Obj* o = x;

  if ( o->meta == NULL )
    o->meta = mk_map(0, NULL);

  return o->meta;
}

Val vs_get_meta(Val x, char* k) {
  Sym* sk = mk_sym(k, NULL, false);

  return get_meta(x, tag(sk));
}

Val vv_get_meta(Val x, Val k) {
  Map* m = meta(x);
  Val v;

  if ( map_get(m, k, &v) )
    return v;

  return NOTHING;
}

Val os_get_meta(void* x, char* k) {
  Sym* sk = mk_sym(k, NULL, false);

  return get_meta(x, tag(sk));
}

Val ov_get_meta(void* x, Val k) {
  Map* m = meta(x);
  Val v;

  if ( map_get(m, k, &v) )
    return v;

  return NOTHING;
}

void vs_set_meta(Val x, char* k, Val v) {
  Sym* sk = mk_sym(k, NULL, false);

  set_meta(x, tag(sk), v);
}

void vv_set_meta(Val x, Val k, Val v) {
  MMEntry* me;

  me = mmap_intern(&MetaData, x, intern_metadata, NULL);

  Map* m = as_map(me->val);
  m = map_set(m, k, v);
  me->val = tag(m);
}

void os_set_meta(void* x, char* k, Val v) {
  Sym* sk = mk_sym(k, NULL, false);

  set_meta(x, tag(sk), v);
}

void ov_set_meta(void* x, Val k, Val v) {
  Obj* o = x;
  Map* m = meta(o);

  m = map_set(m, k, v);
  o->meta = m;
}

bool vs_meta_eq(Val x, char* k, Val v) {
  return get_meta(x, k) == v;
}

bool vv_meta_eq(Val x, Val k, Val v) {
  return get_meta(x, k) == v;
}

bool os_meta_eq(void* x, char* k, Val v) {
  return get_meta(x, k) == v;
}
bool ov_meta_eq(void* x, Val k, Val v) {
  return get_meta(x, k) == v;
}
