#include <stdarg.h>

#include "opcodes.h"
#include "vm.h"
#include "compiler.h"

// internal API
// declarations
// helpers
static ByteCode* bytecode(Vm* vm);
static Values*   values(Vm* vm);
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
static ByteCode* bytecode(Vm* vm) {
  return &compilerChunk(vm)->code;
}

static Values* values(Vm* vm) {
  return &compilerChunk(vm)->vals;
}

static size_t emitInstruction(Vm* vm, OpCode op, ...) {
  size_t argc = opCodeArgc(op);
  va_list va;
  va_start(va, op);

  writeByteCode(bytecode(vm), op);

  if (argc > 0)
    writeByteCode(bytecode(vm), va_arg(va, int));

  va_end(va);

  return bytecode(vm)->count;
}

static size_t addValue(Vm* vm, Value value) {
  return writeValues(values(vm), value);
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
  
  size_t argc = saveXprs(vm, comb->tail, true);

  compileExpression(vm);
  compileExpressions(vm, argc, true);

  return emitInstruction(vm, OP_CALL, argc);
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

  return bytecode(vm)->count;
}

static size_t saveXprs(Vm* vm, List* xprs, bool keepHead) {
  size_t out    = xprs->arity;
  size_t offset = writeValuesN(values(vm), out+keepHead, NULL);

  for (size_t i=out+keepHead; i > 0; i--, xprs=xprs->tail)
    values(vm)->data[offset+i-1] = xprs->head;

  return out;
}

static void saveXpr(Vm* vm, Value xpr) {
  writeValues(values(vm), xpr);
}

static Value unsaveXpr(Vm* vm) {
  return popValues(compilerStack(vm));
}

// special forms
size_t compileQuote(Vm* vm, List* form);
size_t compileDo(Vm* vm, List* form);
size_t compileVar(Vm* vm, List* form);
size_t compileIf(Vm* vm, List* form);

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
