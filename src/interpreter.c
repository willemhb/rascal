#include <stdlib.h>
#include <stdio.h>

#include "util/io.h"

#include "vm.h"
#include "reader.h"
#include "interpreter.h"

// internal API
static bool isLiteral(Value x) {
  if (IS_SYMBOL(x))
    return *AS_SYMBOL(x)->name == ':';

  if (IS_LIST(x))
    return AS_LIST(x)->arity == 0;

  return false;
}

// external API
void initInterpreter(Interpreter* interpreter, Value* vals, size_t nStack) {
  interpreter->sp   = vals;
  interpreter->vp   = vals;
  interpreter->ep   = vals+nStack;
  interpreter->code = NULL;
  interpreter->ip   = NULL;
}

void freeInterpreter(Interpreter* interpreter) {
  interpreter->sp   = NULL;
  interpreter->vp   = NULL;
  interpreter->ep   = NULL;
  interpreter->code = NULL;
  interpreter->ip   = NULL;
}

void resetInterpreter(Interpreter* interpreter, Chunk* code) {
  interpreter->sp   = interpreter->vp;
  interpreter->code = code;
  interpreter->ip   = code->code.data;
}


Value eval(Vm* vm, Value xpr) {
  Value val; Chunk* code;
  
  if (isLiteral(xpr))
    val = xpr;

  else if (IS_SYMBOL(xpr))
    lookupGlobal(vm, AS_SYMBOL(xpr), &val);

  else {
    code = compile(vm, xpr);

    if (code == NULL) // error occured
      val = NUL_VAL;
    else
      val = exec(vm, code);
  }

  return val;
}

void repl(Vm* vm) {
  static const char*  prompt  = "rascal>";

  for (;;) {
    fprintf(stdout, "%s ", prompt);
    Value xpr = read(vm);
    fprintf(stdout, "\n");
    Value val = eval(vm, xpr);
    printValue(stdout, val);
    fprintf(stdout, "\n");
  }
}

Value exec(Vm* vm, Chunk* code) {
  static void* labels[] = {
    [OP_NOTHING] = &&op_nothing,
    [OP_POP]     = &&op_pop,
    [OP_RETURN]  = &&op_return,
    [OP_NUL]     = &&op_nul,
    [OP_TRUE]    = &&op_true,
    [OP_FALSE]   = &&op_false,
    [OP_EMPTY]   = &&op_empty,
    
  };

 dispatch:
  
 op_nothing:

 op_pop:
  
}
