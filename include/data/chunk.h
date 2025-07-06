#ifndef rl_data_chunk_h
#define rl_data_chunk_h

/* Type representing a compiled user function. */

// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
struct Chunk {
  HEAD;

  Env*     vars;
  MutList* vals;
  MutBin*  code;
};

// globals --------------------------------------------------------------------

// forward declarations -------------------------------------------------------
Chunk* mk_chunk(Env* vars, MutList* vals, MutBin* code);
void   dis_chunk(Chunk* chunk);

// initialization -------------------------------------------------------------
void toplevel_init_data_chunk(void);

#endif
