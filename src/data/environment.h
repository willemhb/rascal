#ifndef data_environment_h
#define data_environment_h

#include "data/object.h"

// C types
struct environment {
  HEADER;
  namespace_t* ns;
  vector_t*    binds;
};

typedef enum {
  LOCALENV    = 0x01,
  SCRIPTENV   = 0x02,
  TOPLEVELENV = 0x03
} envfl_t;

// API & 


#endif
