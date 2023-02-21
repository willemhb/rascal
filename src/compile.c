#include "object.h"

#include "compile.h"

// globals --------------------------------------------------------------------
Val Def, Set, Fn, Do, Quote, If;

Sym* Toplevel;

// local helpers 

// API ------------------------------------------------------------------------

Func* compile(Val x) {
  Func* target = new_func(TOPLEVEL, 0, Toplevel, NULL, NULL);

  
}

// initialization -------------------------------------------------------------
void compile_init(void) {
  // special form names -------------------------------------------------------
  Sym* defsym  =get_sym(false, "def");   set_flag(defsym, LITERAL);   Def   = tag(defsym);
  Sym* setsym  =get_sym(false, "set");   set_flag(setsym, LITERAL);   Set   = tag(setsym);
  Sym* fnsym   =get_sym(false, "fn");    set_flag(fnsym, LITERAL);    Fn    = tag(fnsym);
  Sym* dosym   =get_sym(false, "do");    set_flag(dosym, LITERAL);    Do    = tag(dosym);
  Sym* ifsym   =get_sym(false, "if");    set_flag(ifsym, LITERAL);    If    = tag(ifsym);
  Sym* quotesym=get_sym(false, "quote"); set_flag(quotesym, LITERAL); Quote = tag(quotesym);

  // dummy function names -----------------------------------------------------
  Toplevel     =get_sym(false, "*toplevel*");
}
