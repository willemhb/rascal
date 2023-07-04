#ifndef util_buffer_h
#define util_buffer_h

#include "common.h"

// simple dynamic array of binary data ++++++++++++++++++++++++++++++++++++++++
typedef enum {
  BINARY,
  ASCII
} encoding_t;

struct buffer {
  void* data;
  usize cnt, cap;
  int elSize;
  encoding_t encoding;
};

void init_buffer( buffer_t* slf, int elSize, encoding_t encoding );
void free_buffer( buffer_t* slf );
void reset_buffer( buffer_t* slf );
usize resize_buffer( buffer_t* slf, usize n );
usize buffer_write( buffer_t* slf, usize n, void* data );
uhash hash_buffer( buffer_t* slf );
int compare_buffers( buffer_t* x, buffer_t* y, bool eq );


#endif
