#include "eval.h"
#include "object.h"
#include "runtime.h"
#include "compile.h"

// helpers --------------------------------------------------------------------
extern bool is_literal(Val x);

Val toplevel(Val n) {
  
}

Val lookup(Val n) {
  if (Interpreter.fp > Interpreter.fb) {
    List* lenv = LENVRX,* cenv = CENVRX;

    
  }
}

// API ------------------------------------------------------------------------
Val eval(Val x) {
  if (is_literal(x))
    return x;

  else if (has_type(x, SYM)) {
    
  }
}
