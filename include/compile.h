#ifndef rascal_compile_h
#define rascal_compile_h

#include "common.h"

// C types
// globals
extern Value FunSym, VarSym, IfSym, WithSym, QuoteSym, DoSym, UseSym;

// external API
void initCompiler(Vm* vm);
void freeCompiler(Vm* vm);
void startCompiler(Vm* vm, Value xpr);
void resetCompiler(Vm* vm);
void syncCompiler(Vm* vm);

Chunk* compile(Value xpr);

#endif
