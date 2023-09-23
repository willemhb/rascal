#ifndef rascal_compiler_h
#define rascal_compiler_h

#include "common.h"
#include "object.h"

typedef enum {
  TOPLEVEL_CHUNK, // script or repl code
  MODULE_CHUNK,   // module code
  CLOSURE_CHUNK   // user function
} ChunkType;

typedef struct Compiler Compiler;

struct Compiler {
  Compiler* enclosing;
  Chunk*    compiling;
  ChunkType type;
  size_t    nLocals;
  size_t    nUpvalues;
  size_t    scopeDepth;
  Value     locals[UINT8_COUNT];
  Value     upValues[UINT8_COUNT];
};

// external API
void      initCompiler(Compiler* enclosing, Chunk* compiling, ChunkType type);
Compiler* freeCompiler(Compiler* compiler);
bool      compileTree(Compiler* toplevel, Tuple* parseTree);

#endif
