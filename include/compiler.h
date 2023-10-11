#ifndef rascal_compiler_h
#define rascal_compiler_h

#include "common.h"
#include "object.h"

// C types
struct Compiler {
  Chunk* chunk;
  Values stack;
};

// globals
extern Value DoSym, QuoteSym, VarSym, IfSym;

// external API
void initCompiler(Compiler* compiler);
void freeCompiler(Compiler* compiler);
void startCompiler(Compiler* compiler, Value xpr);
void resetCompiler(Compiler* compiler);
void syncCompiler(Compiler* compiler);

Chunk* compile(Vm* vm, Value xpr);

#endif
