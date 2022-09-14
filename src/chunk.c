#include <stdlib.h>

#include "chunk.h"
#include "memory.h"
#include "value.h"

void initChunk( Chunk *chunk )
{
  chunk->code      = ALLOCATE(UInt16Array, 1);
  chunk->lines     = ALLOCATE(Int32Array, 1);
  chunk->constants = ALLOCATE(ValueArray, 1);

  initUInt16Array( chunk->code );
  initInt32Array( chunk->lines );
  initValueArray( chunk->constants );
}

void freeChunk( Chunk *chunk )
{
  finalizeUInt16Array( chunk->code );     chunk->code      = NULL;
  finalizeInt32Array( chunk->lines );     chunk->lines     = NULL;
  finalizeValueArray( chunk->constants ); chunk->constants = NULL;
}

void writeChunk( Chunk* chunk, uint16_t byte, int line ) {
  writeUInt16Array( chunk->code, byte );
  writeInt32Array( chunk->lines, line );
}

int addConstant( Chunk* chunk, Value value )
{
  writeValueArray( chunk->constants, value );
  return chunk->constants->length - 1;
}
