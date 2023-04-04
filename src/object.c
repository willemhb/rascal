#include <string.h>
#include <stdio.h>

#include "object.h"
#include "text.h"
#include "sexpr.h"

#include "memory.h"
#include "number.h"
#include "hashing.h"

// lifetime API ---------------------------------------------------------------
void* new_object(data_type_t* type, usize count, flags fl) {

  object_t* obj = alloc_object(type, count, fl);

  init_object(obj, type, count, fl);

  return obj;
}

void* alloc_object(data_type_t* type, usize count, flags fl) {
  extern object_t* LiveObjects;
  
  void* out;

  if (type->alloc)
    out = type->alloc(type, count, fl);
  
  else
    out = allocate(type->size + count);

  *(object_t*)out = (object_t) { .next=LiveObjects };

  LiveObjects = out;
  
  return out;
}

void init_object(void* self, data_type_t* type, usize count, flags fl) {
  
  *(object_t*)self = (object_t) {
    .hash  =0,
    .flags =fl,
    .hashed=false,
    .frozen=has_flag(type, IMMUTABLE),
    .gray  =true,
    .black =false
  };

  if (type->init)
    type->init(self, type, count, fl);
}

void* copy_object(void* self, usize padding, bool editable) {
  data_type_t* type = type_of(self);
  void* copy;

  if (type->copy)
    copy = type->copy(self, padding);

  else
    copy = duplicate(self, rl_size_of(self) + padding);

  header(copy)->frozen = !editable;

  return copy;
}

void mark_object(void* self) {
  if (!self)
    return;

  object_t* obj = self;

  if (obj->black)
    return;

  obj->black = true;

  data_type_t* type = (data_type_t*)type_of(obj);

  if (type->trace)
    push_gray(obj);

  else
    obj->gray = true;
}

void free_object(void* self) {
  if (!self)
    return;
  
  object_t* obj     = self;
  usize dealloc     = rl_size_of(obj);
  data_type_t* type = type_of(obj);

  if (type->free)
    type->free(obj);

  deallocate(obj, dealloc);
}

// iterator API ---------------------------------------------------------------
void* iter(void* iterable) {
  object_t* obj = iterable;

  if (obj->type == ITERATOR)
    return obj;
  
  data_type_t* type = type_of(iterable);

  if (type->iter)
    return type->iter(obj);

  return NULL;
}

value_t next(void** iterbuf) {
  if (*iterbuf == NULL)
    return STOPITER;

  object_t* obj = *iterbuf;

  if (obj->type == ITERATOR)
    return ((iterator_t*)obj)->type->next(iterbuf);

  data_type_t* type = type_of(obj);

  assert(type->next);

  return type->next(iterbuf);
}

bool hasnext(void* iterable) {
  if (iterable == NULL)
    return false;

  object_t* obj = iterable;

  if (obj->type == ITERATOR)
    return ((iterator_t*)obj)->type->hasnext(iterable);

  data_type_t* type = type_of(obj);
  
  assert(type->next);

  return type->next(iterable);
}
