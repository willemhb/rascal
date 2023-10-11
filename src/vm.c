#include "opcodes.h"
#include "vm.h"

// internal API
static bool isLiteral(Value x) {
  if (IS_SYMBOL(x))
    return *AS_SYMBOL(x)->name == ':';

  if (IS_LIST(x))
    return AS_LIST(x)->arity == 0;

  return false;
}

// external API
void initVm(Vm* vm) {
  initHeap(&vm->heap);
  initEnvironment(&vm->environment);
  initReader(&vm->reader);
  initCompiler(&vm->compiler, NOTHING_VAL);
  initInterpreter(&vm->interpreter, NULL);
}

void freeVm(Vm* vm) {
  freeHeap(&vm->heap);
  freeEnvironment(&vm->environment);
  freeReader(&vm->reader);
  freeCompiler(&vm->compiler);
  freeInterpreter(&vm->interpreter);
}


Value eval(Vm* vm, Value xpr) {
  Value val; Chunk* code;
  
  if (isLiteral(xpr))
    val = xpr;

  else if (IS_SYMBOL(xpr))
    lookupGlobal(&vm->environment, AS_SYMBOL(xpr), &val);

  else {
    code = compile(&vm->compiler, xpr);

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
    Value xpr = read(&vm->reader);
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
