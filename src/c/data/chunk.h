#ifndef data_chunk_h
#define data_chunk_h

#include "data/object.h"

// C types
struct chunk {
  HEADER;
  environment_t* envt;  // namespace
  vector_t*      vals;  // compiled constants
  binary_t*      code;  // instructions
};

// API & utilities
chunk_t* make_chunk(environment_t* envt);

#endif
