#include "opcodes.h"

#include "type.h"
#include "collection.h"
#include "environment.h"
#include "function.h"

#include "vm.h"
#include "compile.h"

// globals
Value FunSym, MacSym, VarSym, IfSym,
  WithSym, QuoteSym, DoSym, UseSym,
  PerformSym, HandleSym, ResumeSym;

// chunk API
extern void freeChunk(void* p);
extern void traceChunk(void* p);

struct Vtable ChunkTable = {
  .valSize=sizeof(Chunk*),
  .objSize=sizeof(Chunk),
  .tag    =OBJ_TAG,
  .free   =freeChunk,
  .trace  =traceChunk
};

struct Type ChunkType = {
  .obj={
    
  },
  
};

Chunk* newChunk(Obj* name, Environment* parent, ScopeType type) {
  size_t ns   = save(2, tag(name), tag(parent));
  Chunk* out  = newObj(&ChunkType, 0, 0);
  ns         += save(1, tag(out));
  out->name   = name;
  out->envt   = newEnvironment(parent, type);
  out->vals   = newAlist(0);
  out->code   = newBinary16(0);
  unsave(ns);
  return out;
}

// helpers
static size_t compileVal(Chunk* chunk, Value val);
static size_t compileVar(Chunk* chunk, Symbol* name);
static size_t compileComb(Chunk* chunk, List* form);

static bool isLiteral(Value val) {
  if (IS(Symbol, val))
    return !getFl(AS(Symbol, val), LITERAL);

  if (IS(List, val))
    return AS(List, val)->arity == 0;

  return true;
}

static size_t compileXpr(Chunk* chunk, Value xpr) {
  if (isLiteral(xpr))
    return compileVal(chunk, xpr);

  else if (IS(Symbol, xpr))
    return compileVar(chunk, AS(Symbol, xpr));

  else
    return compileComb(chunk, AS(List, xpr));
}

static char* getChunkName(Chunk* chunk) {
  if (IS(Symbol, chunk->name))
    return ((Symbol*)chunk->name)->name;

  return ((String*)chunk->name)->data;
}

static bool isIdentifier(Value val) {
  bool out = false;

  if (IS(Symbol, val)) {
    Symbol* s = AS(Symbol, val);
    out       = !getFl(s, LITERAL);
  }

  return out;
}

static bool isMacro(Value val) {
  return IS(Function, val) && getFl(AS(Function, val), MACRO);
}

static bool isSpecialForm(List* form) {
  return IS(Symbol, form->head) && AS(Symbol, form->head)->special != NULL;
}

static Binding* lookupSyntax(Environment* envt, Symbol* name) {
  Binding* out = NULL;

  if (envt && name->special == NULL && !getFl(name, LITERAL)) {
    if (nameSpaceGet(envt->private, name, &out)) {
      if (!isMacro(out->value))
        out = NULL;
    } else if (nameSpaceGet(envt->globals, name, &out)) {
      if (!isMacro(out->value))
        out = NULL;
    }
  }

  return out;
}

static Binding* isMacroCall(Environment* envt, List* form) {
  Binding* out = NULL;

  if (IS(Symbol, form->head))
    out = lookupSyntax(envt, AS(Symbol, form->head));

  return out;
}

static size_t emitInstr(Chunk* code, OpCode op, ...) {
  uint16_t buf[3] = { op, 0, 0 };
  size_t argc = opCodeArgc(op);
  size_t n = 1;
  va_list va; va_start(va, op);

  if (argc > 0)
    buf[n++] = va_arg(va, int);

  if (argc > 0)
    buf[n++] = va_arg(va, int);

  va_end(va);

  return binary16Write(code->code, n, buf);
}

static size_t addValue(Chunk* code, Value val) {
  return alistPush(code->vals, val);
}

static Value popSubXpr(void) {
  return alistPop(&RlVm.compiler.stack);
}

static size_t pushSubXprs(List* form) {
  size_t out = form->arity;
  Value buf[out];

  for (size_t i=out; i > 0; i--, form=form->tail)
    buf[i] = form->head;

  alistWrite(&RlVm.compiler.stack, out, buf);
  return out;
}

static size_t compileVar(Chunk* code, Symbol* name) {
  Binding* bind = lookup(code->envt, name);
  size_t out;

  if (bind == NULL) // create binding, but raise an error if it isn't initialized by runtime
    bind = define(NULL, name, NOTHING, false);

  NsType type = getNsType(bind->ns);

  if (type == GLOBAL_NS)
    out = emitInstr(code, OP_LOADG, bind->offset);

  else if (type == PRIVATE_NS)
    out = emitInstr(code, OP_LOADP, bind->offset);

  else if (type == NONLOCAL_NS)
    out = emitInstr(code, OP_LOADS, bind->offset);

  else
    out = emitInstr(code, OP_LOADU, bind->offset);

  return out;
}

static size_t compileVal(Chunk* code, Value val) {
  size_t out, off;

  if (val == NUL)
    out = emitInstr(code, OP_NUL);

  else if (val == TRUE)
    out = emitInstr(code, OP_TRUE);

  else if (val == FALSE)
    out = emitInstr(code, OP_FALSE);

  else if (val == tag(&EmptyBits))
    out = emitInstr(code, OP_EMPTY_BITS);

  else if (val == tag(&EmptyString))
    out = emitInstr(code, OP_EMPTY_STR);

  else if (val == tag(&EmptyList))
    out = emitInstr(code, OP_EMPTY_LIST);

  else if (val == tag(&EmptyTuple))
    out = emitInstr(code, OP_EMPTY_TUPLE);

  else if (val == tag(&EmptyVector))
    out = emitInstr(code, OP_EMPTY_VEC);

  else if (val == tag(&EmptyMap))
    out = emitInstr(code, OP_EMPTY_MAP);

  else {
    off = addValue(code, val);
    out = emitInstr(code, OP_LOADV, off);
  }

  return out;
}

static size_t compileComb(Chunk* chunk, List* form) {
  size_t out;
  Binding* macrob;

  if (isSpecialForm(form))
    out = AS(Symbol, form->head)->special(chunk, form);

  else if ((macrob=isMacroCall(chunk->envt, form))) {
    Function* macro = AS(Function, macrob->value);
    Value xpr = macroExpand(macro, chunk->envt, form);
    out = compileXpr(chunk, xpr);
  } else {
    
  }

  return out;
}

// special forms
size_t compileQuote(Chunk* chunk, List* form) {
  argco(2, form->arity, "quote");
  return compileVal(chunk, form->tail->head);
}

size_t compileDo(Chunk* chunk, List* form) {
  size_t arity, out;
  Value xpr;

  arity = vargco(2, form->arity, "do");

  if (arity == 2)
    out = compileXpr(chunk, form->tail->head);

  else {
    arity = pushSubXprs(form->tail);

    for (size_t i=0; i<arity; i++) {
      xpr = alistPop(&RlVm.compiler.stack);
      out = compileXpr(chunk, xpr);

      if (i+1 < arity)
        emitInstr(chunk, OP_POP);
    }
  }

  return out;
}

size_t compileIf(Chunk* chunk, List* form) {
  size_t arity, offset1, offset2, offset3;
  Value  test, then, otherwise;

  /* consequent is otpional */
  argcos(2, form->arity, "if", 3, 4);

  arity     = pushSubXprs(form->tail);
  test      = popSubXpr();

  compileXpr(chunk, test);

  offset1   = emitInstr(chunk, OP_JUMPF, 0);
  then      = popSubXpr();

  compileXpr(chunk, then);

  offset2   = emitInstr(chunk, OP_JUMP, 0);
  otherwise = arity == 3 ? NUL : popSubXpr();

  offset3   = compileXpr(chunk, otherwise);

  /* fill in jumps */
  chunk->code->data[offset1-1] = offset2-offset1;
  chunk->code->data[offset2-1] = offset3-offset2;

  return offset3;
}

// external API
Chunk* newChunk(Obj* name, Environment* parent, ScopeType type);

Value  macroExpand(Function* macro, Environment* envt, List* form) {
  size_t nsv;
  Value exp;
  Tuple* sig;

  nsv = save(3, tag(macro), tag(envt), tag(form));

  

  unsave(nsv);
}

Chunk* compile(Obj* name, CompilerState state, Value xpr);

void initSpecialForms(void) {
  
}
