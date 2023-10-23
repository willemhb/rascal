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
// special forms
extern Value DefSym, PutSym, LmbSym, // binding forms
  DoSym, IfSym, QuoteSym, CCallSym,  // miscellaneous forms
  HandleSym, PerformSym, ResumeSym;  // effect forms

// other syntax
extern Value AmpSym;
extern struct Type ChunkType;

// external API
Chunk*   new_chunk(void);
Value    macro_expand(Function* macro, Environment* envt, List* form);
Closure* compile(void* name, CompState state, Value xpr);

void init_syntax(void);

#endif
