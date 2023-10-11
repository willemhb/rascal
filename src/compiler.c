#include <stdarg.h>

#include "opcodes.h"
#include "compiler.h"

// internal API
// declarations
// helpers
static size_t emitInstruction(Compiler* compiler, OpCode op, ...);
static size_t addValue(Compiler* compiler, Value value);
static size_t compileValue(Compiler* compiler, Value value);
static size_t compileCall(Compiler* compiler, List* comb);
static size_t compileExpression(Compiler* compiler);
static size_t compileExpressions(Compiler* compiler, size_t n, bool accumulate);
static void   saveXpr(Compiler* compiler, Value xpr);
static size_t saveXprs(Compiler* compiler, List* xprs, bool keepHead);
static Value  unsaveXpr(Compiler* compiler);

// special forms
size_t compileQuote(Compiler* compiler, List* form);
size_t compileDo(Compiler* compiler, List* form);
size_t compileVar(Compiler* compiler, List* form);

// implementations
static size_t emitInstruction(Compiler* compiler, OpCode op, ...) {
  size_t argc = opCodeArgc(op);
  va_list va;
  va_start(va, op);

  writeByteCode(&compiler->chunk->code, op);

  if (argc > 0)
    writeByteCode(&compiler->chunk->code, va_arg(va, int));

  va_end(va);

  return compiler->chunk->code.count;
}

static size_t addValue(Compiler* compiler, Value value) {
  return writeValues(&compiler->chunk->vals, value);
}

static size_t compileValue(Compiler* compiler, Value value) {
  size_t out;

  if (value == NUL_VAL)
    out = emitInstruction(compiler, OP_NUL);

  else if (value == TRUE_VAL)
    out = emitInstruction(compiler, OP_TRUE);

  else if (value == FALSE_VAL)
    out = emitInstruction(compiler, OP_FALSE);

  else if (value == EMPTY_LIST())
    out = emitInstruction(compiler, OP_EMPTY);

  else
    out = emitInstruction(compiler, OP_VALUE, addValue(compiler, value));

  return out;
}

static size_t compileCall(Compiler* compiler, List* comb) {
  
  size_t argc = saveXprs(compiler, comb->tail, true);

  compileExpression(compiler);
  compileExpressions(compiler, argc, true);

  return emitInstruction(compiler, OP_CALL, argc);
}

static size_t compileExpression(Compiler* compiler) {
  size_t out;

  Value xpr = unsaveXpr(compiler);

  if (IS_LIST(xpr) && xpr != EMPTY_LIST())
    out = compileCall(compiler, AS_LIST(xpr));

  else
    out = compileValue(compiler, xpr);

  return out;
}

static size_t compileExpressions(Compiler* compiler, size_t n, bool accumulate) {
  for (size_t i=0; i<n; i++ ) {
    compileExpression(compiler);

    if (!accumulate && i+1 < n)
      emitInstruction(compiler, OP_POP);
  }

  return compiler->chunk->code.count;
}

static size_t saveXprs(Compiler* compiler, List* xprs, bool keepHead) {
  size_t out    = xprs->arity;
  size_t offset = writeValuesN(&compiler->chunk->vals, out+keepHead, NULL);

  for (size_t i=out+keepHead; i > 0; i--, xprs=xprs->tail)
    compiler->chunk->vals.data[offset+i-1] = xprs->head;

  return out;
}

static void saveXpr(Compiler* compiler, Value xpr) {
  writeValues(&compiler->exprs, xpr);
}

static Value unsaveXpr(Compiler* compiler) {
  return popValues(&compiler->exprs);
}

// special forms

// external API
void initCompiler(Compiler* compiler, Value expression) {
  initValues(&compiler->exprs);

  if (expression != NOTHING_VAL) {
    saveXpr(compiler, expression);
    compiler->chunk = newChunk();
  } else {
    compiler->chunk = NULL;
  }
}

void freeCompiler(Compiler* compiler) {
  freeValues(&compiler->exprs);
  initCompiler(compiler, NOTHING_VAL);
}

Chunk* compile(Compiler* compiler, Value expression) {
  Chunk* out;

  initCompiler(compiler, expression);
  compileExpression(compiler);
  emitInstruction(compiler, OP_RETURN);
  out = compiler->chunk;
  freeCompiler(compiler);

  return out;
}
