#include <string.h>

#include "util/buffer.h"
#include "util/number.h"

#include "runtime.h"

// simple dynamic array of binary data ++++++++++++++++++++++++++++++++++++++++
void init_buffer( buffer_t* slf, int elSize, encoding_t encoding ) {
  slf->elSize   = elSize;
  slf->encoding = encoding;
  reset_buffer(slf);
}

void free_buffer( buffer_t* slf ) {
  deallocate(slf->data, 0, false);
  reset_buffer(slf);
}

void reset_buffer( buffer_t* slf ) {
  slf->cnt = 0;
  slf->cap = 0;
  slf->data = NULL;
}

usize resize_buffer( buffer_t* slf, usize n ) {
  usize pn = n + !!slf->encoding;

  if ( pn > slf->cap || pn < (slf->cap >> 1) ) {
    slf->cap = n ? ceil2(pn) : 0;
    slf->data = reallocate(slf->data, 0, slf->cap * slf->elSize, false);
  }

  slf->cnt = n;
  return slf->cnt;
}

usize buffer_write( buffer_t* slf, usize n, void* data ) {
  usize off = slf->cnt * slf->elSize;
  usize nw = n * slf->elSize;
  resize_buffer(slf, slf->cnt + n);
  memcpy(slf->data + off, data, nw);
  return slf->cnt - n;
}
