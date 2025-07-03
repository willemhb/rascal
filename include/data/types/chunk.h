#ifndef rl_data_types_chunk_h
#define rl_data_types_chunk_h

/* Type representing a compiled user function. */

// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
struct Chunk {
  HEAD;

  Env*    vars;
  Alist*  vals;
  Buf16*  code;
};

// globals --------------------------------------------------------------------

// forward declarations -------------------------------------------------------
Chunk* mk_chunk(Env* vars, Alist* vals, Buf16* code);
void   dis_chunk(Chunk* chunk);

// initialization -------------------------------------------------------------
void   toplevel_init_data_type_chunk(void);

#endif
