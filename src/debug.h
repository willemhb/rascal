#ifndef rascal_debug_h
#define rascal_debug_h

#include "chunk.h"

void showStack( void );
void disassembleChunk( Chunk *chunk, const char *name );
int disassembleInstruction( Chunk *chunk, int offset );

#endif
