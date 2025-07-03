#ifndef rl_data_types_buf16_h
#define rl_data_types_buf16_h

/* Mutable 16-bit binary type (used to represent compiled code). */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
struct Buf16 {
  HEAD;

  Bin16 binary;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Buf16* mk_buf16(void);
void   free_buf16(void* ptr);
int    buf16_write(Buf16* b, ushort_t *xs, int n);

// initialization -------------------------------------------------------------
void toplevel_init_data_type_buf16(void);

#endif
