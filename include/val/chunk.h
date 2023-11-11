#ifndef rl_val_chunk_h
#define rl_val_chunk_h

#include "val/object.h"

/* Compiled code representation & support types. */

/* C types */
/* primarily used to store bytecode so it's defined here. */
struct Binary16 {
  HEADER;
  uint16_t* data;
  size_t    cnt;
  size_t    cap;
};

struct Chunk {
  HEADER;
  Envt     envt;
  Alist    values;
  Binary16 code;
};

/* globals */
extern struct Type ChunkType, Binary16Type;

/* external API */


#endif
