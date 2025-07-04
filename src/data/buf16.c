
/* Implementation for 16-bit mutable buffer type. */
// headers --------------------------------------------------------------------

#include "data/buf16.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void free_buf16(void* ptr);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void free_buf16(void* ptr) {
  Buf16* b = ptr;

  free_bin16(&b->binary);
}

// external -------------------------------------------------------------------
Buf16* mk_buf16(void) {
  Buf16* b = mk_obj(EXP_BUF16, 0); init_bin16(&b->binary);

  return b;
}

int buf16_write(Buf16* b, ushort_t *xs, int n) {
  bin16_write(&b->binary, xs, n);

  return b->binary.count;
}

// initialization -------------------------------------------------------------
void toplevel_init_data_buf16(void) {
  Types[EXP_BUF16] = (ExpTypeInfo) {
    .type     = EXP_BUF16,
    .name     = "buf16",
    .obsize   = sizeof(Buf16),
    .free_fn  = free_buf16
  };
}
