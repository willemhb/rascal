#ifndef rl_val_chunk_h
#define rl_val_chunk_h

#include "val/object.h"

/* Compiled code representation & support types. */

/* C types */
struct Chunk {
  HEADER;
  Envt*   envt;
  MutVec* values;
  MutBin* code;
};

/* globals */
extern struct Type ChunkType, Binary16Type;

/* external API */
Chunk* new_chunk(Envt* envt, MutVec* values, MutBin* code);

#endif
