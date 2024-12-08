#ifndef rl_val_buffer_h
#define rl_val_buffer_h

#include "val/object.h"

#include "util/text.h"

/* Generic binary buffer type */
/* C types */
struct Buffer {
  HEADER;
  // bit fields
  CType  type;
  size8  elsize;
  flags8 encoded;

  // data fields
  size64 cnt, cap;
  byte*  data;
};

/* Globals */

/* API */
#define is_buffer(x) has_type(x, T_BUFFER)
#define as_buffer(x) ((Buffer*)as_obj(x))

Buffer* new_buffer(CType type);
void    init_buffer(State* vm, Buffer* b, CType type);
void    shrink_buffer(Buffer* b, size64 n);
void    grow_buffer(Buffer* b, size64 n);
void    resize_buffer(Buffer* b, size64 n);
void*   buffer_ref(Buffer* b, size64 n);
size64  buffer_add(Buffer* b, word_t d);
size64  buffer_wrt(Buffer* b, size64 n, byte* d);
void    buffer_set(Buffer* b, size64 n, word_t d);

/* Initialization */

#endif
