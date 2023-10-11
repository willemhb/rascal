#ifndef rascal_compiler_h
#define rascal_compiler_h

#include "common.h"
#include "object.h"

// C types
struct Compiler {
  Chunk* chunk;
  Values exprs;
};

// globals
extern Value DoSym, QuoteSym, VarSym;

// external API
void   initCompiler(Compiler* compiler, Value expression);
void   freeCompiler(Compiler* compiler);
Chunk* compile(Compiler* compiler, Value expression);

#endif
