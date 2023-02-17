#include "compile.h"
#include "object.h"

/* internals */
void emit(UserFn *chunk, uint8 instr, ...);

/* globals */
// special form names ---------------------------------------------------------
Val Def, Put, Lmb, Do, Quote, If;

/* initialization */
void compile_init(void) {
  Def   = sym("def");
  Put   = sym("put");
  Lmb   = sym("lmb");
  Do    = sym("do");
  Quote = sym("quote");
  If    = sym("if");
}

