#ifndef rascal_compile_h
#define rascal_compile_h

#include "environment.h"

// C types 
struct Chunk {
  Obj          obj;
  Obj*         name; // name of the module (either a function or a script)
  Environment* envt; // name context
  Alist*       vals;
  Binary16*    code;
};

// globals
extern Value FunSym, VarSym, MacSym,
  IfSym, WithSym, QuoteSym, DoSym, UseSym,
  PerformSym, HandleSym, ResumeSym;

extern struct Type ChunkType;

// external API
Chunk* newChunk(Obj* name, Environment* parent, ScopeType type);
Value  macroexpand(Function* macro, Environment* envt, List* form);
Chunk* compile(Obj* name, CompilerState state, Value xpr);

#endif
