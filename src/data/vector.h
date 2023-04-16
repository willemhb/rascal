#ifndef data_vector_h
#define data_vector_h

#include "data/object.h"

// C types
struct vector {
  HEADER;
  usize    cnt, cap;
  value_t* array;
};

// APIs & utilities
#define   as_vector(x) ((vector_t*)((x) & WVMASK))

vector_t* vector(usize n);
void      reset_vector(vector_t* slf);
usize     vector_push(vector_t* slf, value_t val);
value_t   vector_pop(vector_t* slf);

#endif
