#ifndef data_binary_h
#define data_binary_h

#include "data/object.h"

// C types
struct binary {
  HEADER;
  ARRAY(void);
  int   elsize;
  bool  encoded;
};

// APIs & utilities
// binary
binary_t* binary(usize n, int elsize, bool encoded);
void      init_binary(binary_t* slf, usize n, int elsize, int encoded);
void      reset_binary(binary_t* slf);
usize     binary_write(binary_t* slf, usize cnt, void* dat);

#endif
