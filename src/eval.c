#include "util/io.h"

#include "vm.h"

#include "collection.h"
#include "function.h"

#include "environment.h"
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
      xpr = readLine();
      val = eval(xpr);
      printLine(stdout, val);
    } catch {
      syncVm(&RlVm);
      fprintf(stdout, "\n");
    }
  }
}

Value exec(Closure* code) {
  static void* labels[] = {
    /* miscellaneous */
    [OP_NOTHING] = &&op_nothing, [OP_POP] = &&op_pop,

    /* constant loads */
    [OP_NUL]         = &&op_nul,         [OP_TRUE]       = &&op_true,
    [OP_FALSE]       = &&op_false,       [OP_EMPTY_LIST] = &&op_empty_list,
    [OP_EMPTY_TUPLE] = &&op_empty_tuple, [OP_EMPTY_VEC]  = &&op_empty_vec,
    [OP_EMPTY_MAP]   = &&op_empty_map,   [OP_EMPTY_STR]  = &&op_empty_str,
    [OP_EMPTY_BITS]  = &&op_empty_bits,  [OP_ZERO]       = &&op_zero,
    [OP_ONE]         = &&op_one,
 
    /* loads/stores */
    [OP_LOADI16] = &&op_loadi16, [OP_LOADG16] = &&op_loadg16,
    [OP_LOADV]   = &&op_loadv,
    [OP_LOADS]   = &&op_loads,   [OP_PUTS]    = &&op_puts,
    [OP_LOADU]   = &&op_loadu,   [OP_PUTU]    = &&op_putu,
    [OP_LOADP]   = &&op_loadp,   [OP_PUTP]    = &&op_putp,
    [OP_LOADG]   = &&op_loadg,   [OP_PUTG]    = &&op_putg,

    /* jumps */
    [OP_JUMP]  = &&op_jump, [OP_JUMPT] = &&op_jumpt, [OP_JUMPF] = &&op_jumpf,

    /* function calls */
    [OP_CALL0]  = &&op_call0,  [OP_CALL1]  = &&op_call1,
    [OP_CALL2]  = &&op_call2,  [OP_CALLN]  = &&op_calln,
    [OP_TCALL0] = &&op_tcall0, [OP_TCALL1] = &&op_tcall1,
    [OP_TCALL2] = &&op_tcall2, [OP_TCALLN] = &&olp_tcalln,
    [OP_RETURN] = &&op_return,

    /* miscellaneous */
    [OP_USE]    = &&op_use,
  };

  UpValue* up;
  Value x, y, v, * bx, * by;
  OpCode op;
  int16_t argx, argy;
  uint16_t argc;

  startInterpreter(&RlVm, code);

 fetch:
  op   = *RlVm.exec.ip++;
  argc = opCodeArgc(op);

  if (argc > 0)
    argx = *RlVm.exec.ip++;

  if (argc > 1)
    argy = *RlVm.exec.ip++;

  goto *labels[op];

  /* no-op */
 op_nothing:
  goto fetch;

  /* discard value from stack */
 op_pop:
  pop();
  goto fetch;

  //* constant loads for common values *;
 op_nul:
  push(NUL);
  goto fetch;

 op_true:
  push(TRUE);
  goto fetch;

 op_false:
  push(FALSE);
  goto fetch;

 op_empty_list:
  push(tag(&EmptyList));
  goto fetch;

 op_empty_tuple:
  push(tag(&EmptyTuple));
  goto fetch;

 op_empty_vec:
  push(tag(&EmptyVector));
  goto fetch;

 op_empty_map:
  push(tag(&EmptyMap));
  goto fetch;

 op_empty_str:
  push(tag(&EmptyString));
  goto fetch;

 op_empty_bits:
  push(tag(&EmptyBits));
  goto fetch;

 op_zero:
  push(ZERO);
  goto fetch;

 op_one:
  push(ONE);
  goto fetch;

  /* inlined constant loads for small integers and characters */
 op_loadi16:
  push(tag((Small)argx));
  goto fetch;

 op_loadg16:
  push(tag((Glyph)argx));
  goto fetch;

  /* load value from constant store */
 op_loadv:
  v = RlVm.exec.code->code->vals->data[argx];
  push(v);
  goto fetch;

  /* load/store on stack */
 op_loads:
  v = *peek(RlVm.exec.bp+argx);
  push(v);
  goto fetch;

 op_puts:
  *peek(RlVm.exec.bp+argx) = *peek(-1);
  goto fetch;

  /* load/store upvalue */
 op_loadu:
  up = (UpValue*)RlVm.exec.code->upvals->data[argx];
  v  = up->value == NOTHING ? *peek(up->offset) : up->value;
  push(v);
  goto fetch;

 op_putu:
   up = (UpValue*)RlVm.exec.code->upvals->data[argx];
   bx = up->value == NOTHING ? peek(up->offset) : &up->value;
  *bx = *peek(-1);
  goto fetch;

  /* load/store private */
 op_loadp:
  

  /* load/store global */

  /* function & method calls */
  
}
