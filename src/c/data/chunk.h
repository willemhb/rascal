#ifndef data_chunk_h
#define data_chunk_h

#include "data/object.h"

struct chunk {
  HEADER;
  list_t*   envt;  // linked list mapping names to offsets
  vector_t* vals;  // compiled constants
  binary_t* code;  // instructions
};

#endif
