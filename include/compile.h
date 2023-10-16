#ifndef rascal_compile_h
#define rascal_compile_h

#include "object.h"

// C types
struct Chunk {
  Obj          obj;
  Obj*         name;  // readable name (might be string or symbol)
  Environment* envt;
  Alist*       vals;
  Binary16*    code;
};

// globals
extern Value FunSym, VarSym, IfSym, WithSym, QuoteSym, DoSym, UseSym;

// external API
void initCompiler(Vm* vm);
void freeCompiler(Vm* vm);
void startCompiler(Vm* vm, Value xpr);
void resetCompiler(Vm* vm);
void syncCompiler(Vm* vm);

Chunk* newChunk(Symbol* name);

Chunk* compile(Value xpr);

#endif
