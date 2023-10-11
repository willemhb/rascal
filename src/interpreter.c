#include <stdlib.h>
#include <stdio.h>

#include "util/io.h"

#include "vm.h"
#include "interpreter.h"

// internal API
static bool isLiteral(Value x) {
  if (IS_SYMBOL(x))
    return *AS_SYMBOL(x)->name == ':';

  if (IS_LIST(x))
    return AS_LIST(x)->arity == 0;

  return false;
}

static bool isTruthy(Value x) {
  return x != FALSE && x != NUL;
}

static bool isFalsey(Value x) {
  return x == FALSE || x == NUL;
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

void startInterpreter(Interpreter* interpreter, Chunk* code) {
  interpreter->code = code;
  interpreter->ip   = code->code.data;
}

void resetInterpreter(Interpreter* interpreter) {
  interpreter->sp   = interpreter->vp;
  interpreter->code = NULL;
  interpreter->ip   = NULL;
}

void syncInterpreter(Interpreter* interpreter) {
  (void)interpreter;
}

void  push(Vm* vm, Value value);
void  pushn(Vm* vm, size_t n);
Value pop(Vm* vm);

Value eval(Vm* vm, Value xpr) {
  Value val; Chunk* code;
  
  if (isLiteral(xpr))
    val = xpr;

  else if (IS_SYMBOL(xpr))
    lookupGlobal(vm, AS_SYMBOL(xpr), &val);

  else {
    code = compile(vm, xpr);

    if (code == NULL) // error occured
      val = NUL;
    else
      val = exec(vm, code);
  }

  return val;
}

void repl(Vm* vm) {
  static const char*  prompt  = "rascal>";
  Value xpr, val;

  for (;;) {
    fprintf(stdout, "%s ", prompt);

    try {
      xpr = readLine(vm);
      val = eval(vm, xpr);
      printLine(stdout, val);
    } catch {
      
    }
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
    [OP_VALUE]   = &&op_value,
    [OP_CALL]    = &&op_call,
    [OP_JUMP]    = &&op_jump,
    [OP_JUMPF]   = &&op_jumpf,
    [OP_JUMPT]   = &&op_jumpt,
    [OP_GETGL]   = &&op_getgl,
    [OP_PUTGL]   = &&op_putgl
  };

  Value x, v;
  OpCode op;
  int argx, argc;
  Interpreter* interp = &vm->interpreter;
  Environment* envt = &vm->environment;
  
  startInterpreter(interp, code);

 dispatch:
  op   = *interp->ip++;
  argc = opCodeArgc(op);

  if (argc > 0)
    argx = *interp->ip++;

  goto *labels[op];

 op_nothing:
  goto dispatch;

 op_pop:
  pop(vm);
  goto dispatch;

 op_return:
  v = pop(vm);
  return v;

 op_nul:
  push(vm, NUL);
  goto dispatch;

 op_true:
  push(vm, TRUE);
  goto dispatch;

 op_false:
  push(vm, FALSE);
  goto dispatch;

 op_empty:
  push(vm, EMPTY_LIST());
  goto dispatch;

 op_value:
  v = interp->code->vals.data[argx];
  push(vm, v);
  goto dispatch;

 op_jump:
  interp->ip += argx;
  goto dispatch;

 op_jumpt:
  x = pop(vm);

  if (isTruthy(x))
    interp->ip += argx;

  goto dispatch;

 op_jumpf:
  x = pop(vm);

  if (isFalsey(x))
    interp->ip += argx;

  goto dispatch;

 op_call:

 op_getgl:
  v = envt->globalVals.data[argx];
  push(vm, v);
  goto dispatch;

 op_putgl:
  envt->globalVals.data[argx] = peek(vm, -1);
  goto dispatch;
}
