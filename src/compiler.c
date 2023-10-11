#include "opcodes.h"
#include "vm.h"
#include "compiler.h"

// internal API
// declarations
// helpers
static bool      isSpecialForm(List* form);
static size_t    emitInstruction(Vm* vm, OpCode op, ...);
static size_t    addValue(Vm* vm, Value value);
static size_t    compileValue(Vm* vm, Value value);
static size_t    compileCall(Vm* vm, List* comb);
static size_t    compileExpression(Vm* vm);
static size_t    compileExpressions(Vm* vm, size_t n, bool accumulate);
static void      saveXpr(Vm* vm, Value xpr);
static size_t    saveXprs(Vm* vm, List* xprs, bool keepHead);
static Value     unsaveXpr(Vm* vm);

// special forms
size_t compileQuote(Vm* vm, List* form);
size_t compileDo(Vm* vm, List* form);
size_t compileVar(Vm* vm, List* form);
size_t compileIf(Vm* vm, List* form);

// implementations
static bool isSpecialForm(List* form) {
  return form->arity > 0
    &&   IS_SYMBOL(form->head)
    &&   AS_SYMBOL(form->head)->special != NULL;
}

static size_t emitInstruction(Vm* vm, OpCode op, ...) {
  Compiler* comp = &vm->compiler;
  Chunk* chunk = comp->chunk;
  ByteCode* code = &chunk->code;
  size_t argc = opCodeArgc(op);
  va_list va;
  va_start(va, op);

  writeByteCode(code, op);

  if (argc > 0)
    writeByteCode(code, va_arg(va, int));

  va_end(va);

  return code->count;
}

static size_t addValue(Vm* vm, Value value) {
  Compiler* comp = &vm->compiler;
  Chunk* chunk = comp->chunk;
  Values* vals = &chunk->vals;

  return writeValues(vals, value);
}

static size_t compileValue(Vm* vm, Value value) {
  size_t out;

  if (value == NUL)
    out = emitInstruction(vm, OP_NUL);

  else if (value == TRUE)
    out = emitInstruction(vm, OP_TRUE);

  else if (value == FALSE)
    out = emitInstruction(vm, OP_FALSE);

  else if (value == EMPTY_LIST())
    out = emitInstruction(vm, OP_EMPTY);

  else
    out = emitInstruction(vm, OP_VALUE, addValue(vm, value));

  return out;
}

static size_t compileCall(Vm* vm, List* comb) {
  size_t out;

  if (isSpecialForm(comb))
    out = AS_SYMBOL(comb->head)->special(vm, comb);

  else {
    size_t argc = saveXprs(vm, comb->tail, true);

    compileExpression(vm);
    compileExpressions(vm, argc, true);

    out = emitInstruction(vm, OP_CALL, argc);
  }

  return out;
}

static size_t compileExpression(Vm* vm) {
  size_t out;

  Value xpr = unsaveXpr(vm);

  if (IS_LIST(xpr) && xpr != EMPTY_LIST())
    out = compileCall(vm, AS_LIST(xpr));

  else
    out = compileValue(vm, xpr);

  return out;
}

static size_t compileExpressions(Vm* vm, size_t n, bool accumulate) {
  for (size_t i=0; i<n; i++ ) {
    compileExpression(vm);

    if (!accumulate && i+1 < n)
      emitInstruction(vm, OP_POP);
  }

  return vm->compiler.chunk->code.count;
}

static size_t saveXprs(Vm* vm, List* xprs, bool keepHead) {
  Compiler* comp = &vm->compiler;
  Values* stack = &comp->stack;
  size_t out    = xprs->arity;
  size_t offset = writeValuesN(stack, out+keepHead, NULL);

  for (size_t i=out+keepHead; i > 0; i--, xprs=xprs->tail)
    stack->data[offset+i-1] = xprs->head;

  return out;
}

static void saveXpr(Vm* vm, Value xpr) {
  Compiler* comp = &vm->compiler;
  Values* stack = &comp->stack;
  writeValues(stack, xpr);
}

static Value unsaveXpr(Vm* vm) {
  Compiler* comp = &vm->compiler;
  Values* stk = &comp->stack;

  return popValues(stk);
}

// special forms
size_t compileQuote(Vm* vm, List* form) {
  argco(2, form->arity, "quote");

  return compileValue(vm, form->tail->head);
}

size_t compileDo(Vm* vm, List* form) {
  size_t arity = vargco(1, form->tail->arity, "do");

  if (arity == 1)
    
}

size_t compileVar(Vm* vm, List* form);
size_t compileIf(Vm* vm, List* form) {
  size_t argc = argcos(2, form->tail->arity, "if", 2, 3);
  
}

// external API
void initCompiler(Compiler* compiler) {
  compiler->chunk = NULL;

  initValues(&compiler->stack);
}

void freeCompiler(Compiler* compiler) {
  freeValues(&compiler->stack);
}

void startCompiler(Compiler* compiler, Value xpr) {
  writeValues(&compiler->stack, xpr);

  compiler->chunk = newChunk();
}

void resetCompiler(Compiler* compiler) {
  freeCompiler(compiler);
  initCompiler(compiler);
}

Chunk* compile(Vm* vm, Value xpr) {
  resetCompiler(compiler(vm));
  startCompiler(compiler(vm), xpr);
  compileExpression(vm);
  emitInstruction(vm, OP_RETURN);

  return compiler(vm)->chunk;
}
