#ifndef rl_buf_h
#define rl_buf_h

#include "val/val.h"

// wrapper around a binary object
struct Buf16 {
  HEAD;
  Bin16 binary;
};

// buf16 API
Buf16* mk_buf16(RlState* rls);
Buf16* mk_buf16_s(RlState* rls);
void free_buf16(RlState* rls, void* ptr);
int buf16_write(RlState* rls, Buf16* b, ushort_t *xs, int n);

#endif
