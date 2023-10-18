#ifndef rascal_compile_h
#define rascal_compile_h

#include "environment.h"

// C types
struct Chunk {
  Obj          obj;
  Alist*       vals;
  Binary16*    code;
};

// globals
extern Value FunSym, MacSym, VarSym, PutSym,
  IfSym, WithSym, QuoteSym, DoSym, UseSym,
  PerformSym, HandleSym, ResumeSym;

extern struct Type ChunkType;

// external API
Chunk*   new_chunk(Environment* parent, ScopeType type);
Value    macro_expand(Function* macro, Environment* envt, List* form);
Closure* compile(void* name, CompState state, Value xpr);

void     init_special_forms(void);

#endif
