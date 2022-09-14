#ifndef rascal_chunk_h
#define rascal_chunk_h

#include "common.h"
#include "value.h"
#include "object.h"
#include "array.h"

typedef enum
  {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_SAME,
    OP_GREATER,
    OP_LESS,
    OP_NEGATE,
    OP_NOT,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_RETURN,
  } OpCode;

typedef struct
{
  HEADER;
  UInt16Array *code;
  Int32Array  *lines;
  ValueArray  *constants;
} Chunk;

void initChunk(Chunk *chunk );
void freeChunk(Chunk *chunk );
void writeChunk( Chunk *chunk, uint16_t byte, int line );
int  addConstant( Chunk *chunk, Value value );

#endif
