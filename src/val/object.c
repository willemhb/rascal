#include "vm/memory.h"

#include "val/type.h"
#include "val/map.h"
#include "val/object.h"

// casts, predicates, accessors, bit twiddling
Type* type_of_obj(void* obj) {
  assert(obj);
  Obj* o = obj;
  return o->type;
}

size_t size_of_obj(void* obj) {
  Obj* o = obj;
  
  if (o->type->vtable->sizefn)
    return o->type->vtable->sizefn(obj);

  return o->type->vtable->osize;
}

bool has_type_obj(void* obj, Type* type) {
  assert(obj);
  Obj* o = obj;

  return is_instance(o->type, type);
}


void mark_obj(void* obj) {
  if (obj) {
    if (get_mfl(obj, NOTRACE))
      return;

    if (get_mfl(obj, BLACK))
      return;

    set_mfl(obj, BLACK);
    Type* t = type_of(obj);

    if (t->vtable->tracefn)
      add_to_grays(&Ctx, obj);

    else
      del_mfl(obj, GRAY);
  }
}

void trace_obj(void* obj) {
  if (obj) {
    Type* t = type_of(obj);
    
    if (t->vtable->tracefn)
      t->vtable->tracefn(obj);

    del_mfl(obj, GRAY);
  }
}

bool get_mfl_obj(void* obj, flags_t mfl) {
  assert(obj);
  Obj* o = obj;
  return !!(o->memfl & mfl);
}

bool set_mfl_obj(void* obj, flags_t mfl) {
  assert(obj);
  Obj* o = obj;
  bool out = !!(o->memfl & mfl);
  o->memfl |= mfl;

  return out;
}

bool del_mfl_obj(void* obj, flags_t mfl) {
  assert(obj);
  Obj* o = obj;
  bool out = get_mfl_obj(obj, mfl);
  o->memfl &= ~mfl;

  return out;
}

bool get_fl_obj(void* obj, flags_t fl) {
  assert(obj);
  Obj* o = obj;
  return !!(o->flags & fl);
}

bool set_fl_obj(void* obj, flags_t fl) {
  assert(obj);
  Obj* o = obj;
  bool out = !!(o->flags & fl);
  o->flags |= fl;

  return out;
}

bool del_fl_obj(void* obj, flags_t fl) {
  assert(obj);
  Obj* o = obj;
  bool out = !!(o->flags & fl);
  o->flags &= ~fl;

  return out;
}

// metadata
Dict* get_meta_dict_obj(void* obj) {
  assert(obj);
  Obj* o = obj;
  return o->meta;
}

Dict* set_meta_dict_obj(void* obj, Dict* meta) {
  assert(obj);
  Obj* o = obj;
  o->meta = meta;
  return meta;
}

Value get_meta_obj(void* obj, Value key) {
  assert(obj);
  Obj* o = obj;
  return dict_get(o->meta, key);
}

Value set_meta_obj(void* obj, Value key, Value val) {
  assert(obj);
  Obj* o = obj;
  o->meta = dict_set(o->meta, key, val);
  return val;
}

Dict* join_meta_obj(void* obj, Dict* d) {
  assert(obj);
  Obj* o = obj;
  o->meta = join_dicts(o->meta, d);
  return o->meta;
}

// lifetime methods
void* new_obj(Type* type, flags_t flags, flags_t memfl, size_t extra) {
  Obj* out = allocate(&Ctx, type->vtable->osize + extra);
  init_obj(out, type, flags, memfl);

  return out;
}

void init_obj(void* obj, Type* type, flags_t flags, flags_t memfl) {
  Obj* slf = obj;
  add_to_heap(&Ctx, slf);

  slf->hash  = 0;
  slf->type  = type;
  slf->meta  = &EmptyDict;
  slf->memfl = memfl | GRAY;
  slf->flags = flags;
}

void finalize_obj(void* obj) {
  if (obj) {
    Obj* slf = obj;
    
    if (get_mfl(slf, NOFREE))
      return;

    if (slf->type->vtable->finalizefn)
      slf->type->vtable->finalizefn(slf);
  }
}

void free_obj(void* obj) {
  if (obj) {
    if (get_mfl(obj, NOSWEEP))
      return;
    
    deallocate(&Ctx, obj, size_of_obj(obj));
  }
}
