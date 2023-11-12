#include <stdarg.h>

#include "util/collection.h"

#include "vm/memory.h"

#include "val/symbol.h"
#include "val/map.h"
#include "val/func.h"
#include "val/type.h"
#include "val/alist.h"

/* Globals */
/* Alist type */
extern void trace_alist(void* obj);
extern void finalize_alist(void* obj);

INIT_OBJECT_TYPE(Alist, NULL, trace_alist, finalize_alist);

/* Objects type */
extern void trace_objects(void* obj);
extern void finalize_objects(void* obj);

INIT_OBJECT_TYPE(Objects, NULL, trace_objects, finalize_objects);

/* External APIs */
/* Alist API */
Alist* new_alist(void) {
  Alist* out = new_obj(&AlistType, 0, 0, 0);
  init_alist(out);
  return out;
}

void init_alist(Alist* arr) {
  arr->data = NULL;
  arr->cnt  = 0;
  arr->cap  = 0;
}

void free_alist(Alist* arr) {
  deallocate(NULL, arr->data, 0);
  init_alist(arr);
}

void resize_alist(Alist* arr, size_t new_cnt) {
  if (new_cnt == 0)
    free_alist(arr);

  else {
    size_t old_cnt, old_cap, new_cap;

    old_cnt = arr->cnt;
    old_cap = arr->cap;
    new_cap = pad_alist_size(old_cnt, new_cnt, old_cap, false);

    if (new_cap != old_cap) {
      arr->data = reallocate(NULL,
                             arr->data,
                             old_cap*sizeof(Value),
                             new_cap*sizeof(Value));
      arr->cap  = new_cap;
    }

    arr->cnt = new_cnt;
  }
}

size_t alist_push(Alist* arr, Value x) {
  size_t out;

  out = arr->cnt;
  resize_alist(arr, out+1);
  arr->data[out] = x;

  return out;
}

size_t alist_write(Alist* arr, size_t n, Value* data) {
  size_t out = arr->cnt;

  resize_alist(arr, out+n);

  if (data != NULL)
    memcpy(arr->data+out, data, n * sizeof(Value));

  return out;
}

size_t alist_pushn(Alist* arr, size_t n, ...) {
  Value buf[n];
  va_list va;

  va_start(va, n);

  for (size_t i=0; i<n; i++)
    buf[i] = va_arg(va, Value);

  va_end(va);

  return alist_write(arr, n, buf);
}

Value alist_pop(Alist* arr) {
  Value out;
  
  assert(arr->cnt > 0);
  out = arr->data[arr->cnt-1];
  resize_alist(arr, arr->cnt-1);

  return out;
}

Value alist_popn(Alist* arr, size_t n) {
  Value out;

  assert(n <= arr->cnt);
  out = arr->data[arr->cnt-n];
  resize_alist(arr, arr->cnt-n);

  return out;
}

/* Objects API */
Objects* new_objects(void) {
  Objects* out = new_obj(&ObjectsType, 0, 0, 0);
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
