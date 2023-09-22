#ifndef rascal_compiler_h
#define rascal_compiler_h

#include "common.h"
#include "object.h"

typedef struct {
  Chunk* chunk;
  
} Compiler;

bool compile(char* source, Chunk* chunk);

#endif
