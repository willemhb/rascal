#ifndef data_vector_h
#define data_vector_h

#include "data/object.h"

// C types
struct vector {
  HEADER;
  ARRAY(value_t);
};

// APIs & utilities
#define   is_vector(x) (value_type(x) == VECTOR)
#define   as_vector(x) ((vector_t*)(((value_t)(x)) & WVMASK))

vector_t* vector(usize n);
void      reset_vector(vector_t* slf, usize n);
void      init_vector(vector_t* slf, usize n);
usize     vector_push(vector_t* slf, value_t val);
value_t   vector_pop(vector_t* slf);

#endif
