#include "vm/memory.h"

#include "val/type.h"
#include "val/map.h"
#include "val/object.h"

// casts, predicates, accessors, bit twiddling
bool is_obj(Value x) {
  return tag_of(x) == OBJ_TAG;
}

Obj* as_obj(Value x) {
  return (Obj*)untag_48(x);
}

size_t size_of_obj(Obj* obj) {
  if (obj->type->vtable->size)
    return obj->type->vtable->size(obj);

  return obj->type->vtable->osize;
}

bool get_obj_fl(Obj* obj, flags_t fl) {
  return !!(obj->flags & fl);
}

bool set_obj_fl(Obj* obj, flags_t fl) {
  bool out    = get_obj_fl(obj, fl);
  obj->flags |= fl;

  return out;
}

bool del_obj_fl(Obj* obj, flags_t fl) {
  bool out    = get_obj_fl(obj, fl);
  obj->flags &= ~fl;

  return out;
}

bool get_obj_mfl(Obj* obj, flags_t mfl) {
  return !!(obj->memfl & mfl);
}

bool set_obj_mfl(Obj* obj, flags_t mfl) {
  bool out    = get_obj_mfl(obj, mfl);
  obj->memfl |= mfl;

  return out;
}

bool del_obj_mfl(Obj* obj, flags_t mfl) {
  bool out    = get_obj_mfl(obj, mfl);
  obj->memfl &= ~mfl;

  return out;
}

bool obj_is_black(Obj* obj) {
  return get_obj_mfl(obj, BLACK);
}

bool obj_is_gray(Obj* obj) {
  return get_obj_mfl(obj, GRAY);
}

bool obj_is_notrace(Obj* obj) {
  return get_obj_mfl(obj, NOTRACE);
}

bool obj_is_nosweep(Obj* obj) {
  return get_obj_mfl(obj, NOSWEEP);
}

bool obj_is_nofree(Obj* obj) {
  return get_obj_mfl(obj, NOFREE);
}

void obj_mark_black(Obj* obj) {
  set_obj_mfl(obj, BLACK);
}

void obj_unmark_black(Obj* obj) {
  del_obj_mfl(obj, BLACK);
}

void obj_mark_gray(Obj* obj) {
  set_obj_mfl(obj, GRAY);
}

void obj_unmark_gray(Obj* obj) {
  del_obj_mfl(obj, GRAY);
}

void mark_obj(Obj* obj) {
  if (obj) {
    if (obj_is_nosweep(obj))
      return;

    if (obj_is_black(obj))
      return;

    obj_mark_black(obj);

    if (obj->type->vtable->trace)
      add_to_grays(&Ctx, obj);

    else
      obj_unmark_gray(obj);
  }
}

void unmark_obj(Obj* obj) {
  if (obj) {
    obj_unmark_black(obj);
    obj_mark_gray(obj);
  }
}

// metadata
Dict* get_obj_meta(Obj* obj) {
  return obj->meta;
}

void set_obj_meta(Obj* obj, Dict* meta) {
  obj->meta = meta;
}

void put_obj_meta(Obj* obj, Value key, Value val) {
  obj->meta = dict_set(obj->meta, key, val);
}
void merge_obj_meta(Obj* obj, Dict* meta) {
  obj->meta = dict_join(obj->meta, meta);
}

// lifetime methods
Obj* new_obj(Type* type, flags_t flags, flags_t memfl, size_t extra) {
  Obj* out = allocate(&Ctx, type->vtable->osize + extra);
  init_obj(out, type, flags, memfl);

  return out;
}

void init_obj(Obj* slf, Type* type, flags_t flags, flags_t memfl) {
  add_to_heap(&Ctx, slf);

  slf->hash  = 0;
  slf->type  = type;
  slf->meta  = &EmptyDict;
  slf->memfl = memfl | GRAY;
  slf->flags = flags;
}

void trace_obj(Obj* obj) {
  if (obj) {
    if (obj->type->vtable->trace)
      obj->type->vtable->trace(obj);

    obj_unmark_gray(obj);
  }
}

void finalize_obj(Obj* obj) {
  if (obj) {
    if (obj_is_nofree(obj))
      return;

    if (obj->type->vtable->finalize)
      obj->type->vtable->finalize(obj);
  }
}

void free_obj(Obj* obj) {
  if (obj) {
    if (obj_is_nosweep(obj))
      return;

    deallocate(&Ctx, obj, size_of_obj(obj));
  }
}
