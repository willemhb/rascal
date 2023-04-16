#ifndef data_chunk_h
#define data_chunk_h

#include "data/object.h"

typedef enum {
  LAMBDA = 0x01,
  SCRIPT = 0x02
} chunkfl_t;

struct chunk {
  HEADER;
  namespace_t* ns;    // local namespace
  vector_t*    vals;  // compiled constants
  binary_t*    code;  // instructions
};

#endif
