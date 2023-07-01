#include "util/alist.h"
#include "util/number.h"

#include "object.h"
#include "runtime.h"

// dynamic array of tagged data +++++++++++++++++++++++++++++++++++++++++++++++
void init_values( values_t* slf ) {
  slf->cnt = 0;
  slf->cap = 0;
  slf->data = NULL;
}

void free_values( values_t* slf ) {
  deallocate(slf->data, 0, false);
  init_values(slf);
}

void reset_values( values_t* slf ) {
  free_values(slf);
}

usize resize_values( values_t* slf, usize n ) {
  if ( n > slf->cap || n < (slf->cap >> 1) ) {
    slf->cap = n ? ceil2(n) : 0;
    slf->data = reallocate(slf->data, 0, slf->cap * sizeof(value_t), false);
  }

  slf->cnt = n;
  return slf->cnt;
}

usize values_push( values_t* slf, value_t val ) {
  resize_values(slf, slf->cnt+1);
  slf->data[slf->cnt-1] = val;
  return slf->cnt-1;
}

value_t values_pop( values_t* slf ) {
  value_t out = slf->data[slf->cnt-1];
  resize_values(slf, slf->cnt-1);
  return out;
}

void trace_values( values_t* slf ) {
  for ( usize i=0; i<slf->cnt; i++ )
    mark(slf->data[i]);
}

// dynamic array of objects +++++++++++++++++++++++++++++++++++++++++++++++++++
void init_objects( objects_t* slf ) {
  slf->cnt = 0;
  slf->cap = 0;
  slf->data = NULL;
}

void free_objects( objects_t* slf ) {
  deallocate(slf->data, 0, false);
  init_objects(slf);
}

void reset_objects( objects_t* slf ) {
  free_objects(slf);
}

usize resize_objects( objects_t* slf, usize n ) {
  if ( n > slf->cap || n < (slf->cap >> 1) ) {
    slf->cap = n ? ceil2(n) : 0;
    slf->data = reallocate(slf->data, 0, slf->cap * sizeof(void*), false);
  }

  slf->cnt = n;
  return slf->cnt;
}

usize objects_push( objects_t* slf, void* obj ) {
  resize_objects(slf, slf->cnt+1);
  slf->data[slf->cnt-1] = obj;
  return slf->cnt-1;
}

void* objects_pop( objects_t* slf ) {
  void* out = slf->data[slf->cnt-1];
  resize_objects(slf, slf->cnt-1);
  return out;
}

void trace_objects( objects_t* slf ) {
  for ( usize i=0; i<slf->cnt; i++ )
    mark(slf->data[i]);
}
