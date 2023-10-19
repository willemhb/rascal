#include "util/io.h"

#include "runtime.h"

#include "collection.h"
#include "function.h"

#include "environment.h"
#include "compile.h"
#include "read.h"
#include "print.h"
#include "eval.h"

// internal API
static bool is_literal(Value x) {
  if (IS(Symbol, x))
    return get_fl(as_obj(x), LITERAL);

  if (is_list(x))
    return as_list(x)->arity == 0;

  return false;
}

static bool is_truthy(Value x) {
  return x != FALSE && x != NUL;
}

static bool isFalsey(Value x) {
  return x == FALSE || x == NUL;
}

// external API
void initInterpreter(Vm* vm) {
  vm->exec.sp    = 0;
  vm->exec.fp    = 0;
  vm->exec.bp    = 0;
  vm->exec.code  = NULL;
  vm->exec.ip    = NULL;
  vm->stackBase  = Stack;
  vm->stackEnd   = &Stack[N_STACK];
  vm->framesBase = Frames;
  vm->framesEnd  = &Frames[N_FRAME];
}

void freeInterpreter(Vm* vm) {
  (void)vm;
}

void startInterpreter(Vm* vm, Closure* code) {
  vm->exec.code = code;
  vm->exec.ip   = code->code->code->data;
}

void resetInterpreter(Vm* vm) {
  vm->exec.sp     = 0;
  vm->exec.fp     = 0;
  vm->exec.bp     = 0;
  vm->exec.upVals = NULL;
  vm->exec.code   = NULL;
  vm->exec.ip     = NULL;
}

void syncInterpreter(Vm* vm) {
  (void)vm;
}

Value eval(Value xpr) {
  Value val;
  Closure* code;

  if (isLiteral(xpr))
    val = xpr;

  else if (IS(Symbol, xpr)) {
    Binding* b = lookup(NULL, AS(Symbol, xpr));
    require(b != NULL, "eval", "unbound symbol `%s`", AS(Symbol, xpr)->name);
    val = b->value;
  }

  else {
    code = compile("<toplevel>", COMPILER_REPL, xpr);
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
      xpr = read_line();
      val = eval(xpr);
      printLine(stdout, val);
    } catch {
      sync_vm(&RlVm);
      fprintf(stdout, "\n");
    }
  }
}
