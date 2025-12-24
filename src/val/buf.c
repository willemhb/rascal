#include "val/buf.h"
#include "vm.h"

// Type object
Type Buf16Type = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_BUF16,
  .obsize   = sizeof(Buf16),
  .free_fn  = free_buf16
};

// buf16 API
Buf16* mk_buf16(RlState* rls) {
  Buf16* b = mk_obj(rls, &Buf16Type, 0);
  init_bin16(rls, &b->binary);
  return b;
}

Buf16* mk_buf16_s(RlState* rls) {
  Buf16* out = mk_buf16(rls);
  push(rls, tag_obj(out));

  return out;
}

int buf16_write(RlState* rls, Buf16* b, ushort_t *xs, int n) {
  bin16_write(rls, &b->binary, xs, n);

  return b->binary.count;
}

void free_buf16(RlState* rls, void* ptr) {
  Buf16* b = ptr;

  free_bin16(rls, &b->binary);
}
