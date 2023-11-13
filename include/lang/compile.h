#ifndef rl_lang_compile_h
#define rl_lang_compile_h

#include "lang/envt.h"

// C types
// globals
// special forms
extern Value DefSym, FnSym, PutSym,  // binding forms
  DoSym, IfSym, QuoteSym,            // local control forms
  HandleSym, PerformSym,             // effect forms
  CCallSym;                          // miscellaneous forms

// other syntax
extern Value AmpSym;

/* External API */
Chunk* compile_toplevel(Value x);
Chunk* compile_sequence(List* exprs);

void   init_syntax(void);

#endif
