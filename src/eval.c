#include "util/io.h"

#include "vm.h"
#include "environment.h"
#include "collection.h"
#include "compile.h"
#include "read.h"
#include "print.h"
#include "eval.h"

// internal API
static bool isLiteral(Value x) {
  if (IS(Symbol, x))
    return getFl(AS(Obj, x), LITERAL);

  if (IS(List, x))
    return AS(List, x)->arity == 0;

  return false;
}

static bool isTruthy(Value x) {
  return x != FALSE && x != NUL;
}

static bool isFalsey(Value x) {
  return x == FALSE || x == NUL;
}

// external API
Value eval(Value xpr) {
  Value val; Chunk* code;
  
  if (isLiteral(xpr))
    val = xpr;

  else if (IS(Symbol, xpr)) {
    Binding* b = lookup(NULL, AS(Symbol, xpr));
    require(b != NULL, "eval", "unbound symbol `%s`", AS(Symbol, xpr)->name);
    val = b->value;
  }

  else {
    code = compile(xpr);
    val = exec(code);
  }

  return val;
}

void repl(void) {
  static const char*  prompt  = "rascal>";
  Value xpr, val;

  for (;;) {
    fprintf(stdout, "%s ", prompt);

    try {
      xpr = readLine();
      val = eval(xpr);
      printLine(stdout, val);
    } catch {
      syncVm(&RlVm);
      fprintf(stdout, "\n");
    }
  }
}

Value exec(void* code) {
  static void* labels[] = {
    [OP_NOTHING] = &&op_nothing,
    [OP_POP]     = &&op_pop,
    [OP_RETURN]  = &&op_return,
    [OP_NUL]     = &&op_nul,
    [OP_TRUE]    = &&op_true,
    [OP_FALSE]   = &&op_false,
    [OP_CALL]    = &&op_call,
    [OP_JUMP]    = &&op_jump,
    [OP_JUMPF]   = &&op_jumpf,
    [OP_JUMPT]   = &&op_jumpt,
  };

  Value x, v;
  OpCode op;
  int argx, argc;
  
 dispatch:
  op   = *RlVm->ip++;
  argc = opCodeArgc(op);

  if (argc > 0)
    argx = *interp->ip++;

  goto *labels[op];

 op_nothing:
  goto dispatch;

 op_pop:
  pop();
  goto dispatch;

 op_return:
  v = pop();
  return v;

 op_nul:
  push(NUL);
  goto dispatch;

 op_true:
  push(TRUE);
  goto dispatch;

 op_false:
  push(FALSE);
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
