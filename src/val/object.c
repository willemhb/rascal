#include <stdarg.h>

#include "util/collection.h"

#include "vm/memory.h"

#include "val/table.h"
#include "val/symbol.h"
#include "val/func.h"
#include "val/type.h"
#include "val/object.h"

/* Globals */
/* Objects type */
extern void trace_objects(void* obj);
extern void finalize_objects(void* obj);

INIT_OBJECT_TYPE(Objects,
                 .tracefn=trace_objects,
                 .finalizefn=finalize_objects);

/* External APIs */
/* Objects API */
Objects* new_objects(void) {
  Objects* out = new_obj(&ObjectsType, 0, EDITP, 0);
  init_objects(out);
  return out;
}

void init_objects(Objects* arr) {
  arr->data = NULL;
  arr->cnt  = 0;
  arr->cap  = 0;
}

void free_objects(Objects* arr) {
  deallocate(NULL, arr->data, 0);
  init_objects(arr);
}

void resize_objects(Objects* arr, size_t new_cnt) {
  if (new_cnt == 0)
    free_objects(arr);

  else {
    size_t old_cnt, old_cap, new_cap;

    old_cnt = arr->cnt;
    old_cap = arr->cap;
    new_cap = pad_alist_size(old_cnt, new_cnt, old_cap, false);

    if (new_cap != old_cap) {
      arr->data = reallocate(NULL,
                             arr->data,
                             old_cap*sizeof(Obj*),
                             new_cap*sizeof(Obj*));
      arr->cap  = new_cap;
    }

    arr->cnt = new_cnt;
  }
}

size_t objects_push(Objects* arr, Obj* x) {
  size_t out;

  out = arr->cnt;
  resize_objects(arr, out+1);
  arr->data[out] = x;

  return out;
}

size_t objects_write(Objects* arr, size_t n, Obj** data) {
  size_t out = arr->cnt;

  resize_objects(arr, out+n);

  if (data != NULL)
    memcpy(arr->data+out, data, n * sizeof(Obj*));

  return out;
}

size_t objects_pushn(Objects* arr, size_t n, ...) {
  Obj* buf[n];
  va_list va;

  va_start(va, n);

  for (size_t i=0; i<n; i++)
    buf[i] = va_arg(va, Obj*);

  va_end(va);

  return objects_write(arr, n, buf);
}

Obj* objects_pop(Objects* arr) {
  Obj* out;
  
  assert(arr->cnt > 0);
  out = arr->data[arr->cnt-1];
  resize_objects(arr, arr->cnt-1);

  return out;
}

Obj* objects_popn(Objects* arr, size_t n) {
  Obj* out;

  assert(n <= arr->cnt);
  out = arr->data[arr->cnt-n];
  resize_objects(arr, arr->cnt-n);

  return out;
}

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
