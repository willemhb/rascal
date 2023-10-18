#include "opcodes.h"

#include "type.h"
#include "collection.h"
#include "environment.h"
#include "function.h"

#include "vm.h"
#include "eval.h"
#include "compile.h"

// globals
Value FunSym, MacSym, VarSym, IfSym,
  WithSym, QuoteSym, DoSym, UseSym,
  PerformSym, HandleSym, ResumeSym;

// chunk API
extern void free_chunk(void* p);
extern void trace_chunk(void* p);

struct Vtable ChunkTable = {
  .val_size=sizeof(Chunk*),
  .obj_size=sizeof(Chunk),
  .tag     =OBJ_TAG,
  .free    =free_chunk,
  .trace   =trace_chunk
};

Function ChunkCtor = {
  .obj={
    .type    =&FunctionType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .gray    =true,
  }
};

Type ChunkType = {
  .obj={
    .type =&TypeType,
    .annot=&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
  },
  .parent =&TermType,
  .v_table=&ChunkTable,
  .ctor   =&ChunkCtor,
  .idno   =CHUNK,
};

Chunk* new_chunk(Obj* name, Environment* parent, ScopeType type) {
  Chunk* out;
  size_t ns;

  ns          = save(2, tag(name), tag(parent));
  out         = new_obj(&ChunkType, 0, 0);
  ns         += save(1, tag(out));
  out->name   = name;
  out->envt   = new_environment(parent, type);
  out->vals   = new_alist(0);
  out->code   = new_binary16(0);

  unsave(ns);

  return out;
}

// special forms
size_t fun_form(List* form);
size_t mac_form(List* form);
size_t var_form(List* form);
size_t put_form(List* form);
size_t if_form(List* form);
size_t with_form(List* form);
size_t quote_form(List* form);
size_t do_form(List* form);
size_t use_form(List* form);
size_t perform_form(List* form);
size_t handle_form(List* form);

// helpers
static void save_compiler_state(void) {
  if (RlVm.compiler.state != COMPILER_READY) {
    RlVm.compiler.depth++;
    alist_push(&RlVm.compiler.stack, tag((int)RlVm.compiler.state));
    alist_push(&RlVm.compiler.stack, tag(RlVm.compiler.tail_pos));
    alist_push(&RlVm.compiler.stack, tag(RlVm.compiler.chunk));
  }
}

static void restore_compiler_state(void) {
  if (RlVm.compiler.depth > 0) {
    RlVm.compiler.chunk    = as_chunk(alist_pop(&RlVm.compiler.stack));
    RlVm.compiler.tail_pos = as_bool(alist_pop(&RlVm.compiler.stack));
    RlVm.compiler.state    = as_small(alist_pop(&RlVm.compiler.stack));
    RlVm.compiler.depth--;
  } 
}

static size_t compile_val(Value val);
static size_t compile_var(Symbol* name);
static size_t compile_comb(List* form);

static bool is_literal(Value val) {
  if (is_sym(val))
    return !get_fl(as_sym(val), LITERAL);

  if (is_list(val))
    return as_list(val)->arity == 0;

  return true;
}

static size_t compile_xpr(Value xpr) {
  if (is_literal(xpr))
    return compile_val(xpr);

  else if (is_sym(xpr))
    return compile_var(as_sym(xpr));

  else
    return compile_comb(as_list(xpr));
}

static char* get_chunk_name(void) {
  Chunk* chunk = RlVm.compiler.chunk;

  if (is_sym(chunk->name))
    return as_sym(chunk->name)->name;

  return as_str(chunk->name)->data;
}

static bool is_identifier(Value val) {
  return is_sym(val) && !get_fl(as_sym(val), LITERAL);
}

static bool is_special_form(List* form) {
  return is_sym(form->head) && as_sym(form->head)->special != NULL;
}

static Binding* is_macro_call(List* form) {
  Binding* out = NULL;

  if (is_sym(form->head))
    out = lookup_syntax(RlVm.compiler.chunk->envt, as_sym(form->head));

  return out;
}

static size_t emit_instr(Chunk* code, OpCode op, ...) {
  uint16_t buf[3] = { op, 0, 0 };
  size_t argc = opCodeArgc(op);
  size_t n = 1;
  va_list va; va_start(va, op);

  if (argc > 0)
    buf[n++] = va_arg(va, int);

  if (argc > 0)
    buf[n++] = va_arg(va, int);

  va_end(va);

  return binary16_write(code->code, n, buf);
}

static size_t add_value(Chunk* code, Value val) {
  return alist_push(code->vals, val);
}

static Value pop_subxpr(void) {
  return alist_pop(&RlVm.compiler.stack);
}

static size_t push_subxprs(List* form) {
  size_t out = form->arity;
  Value buf[out];

  for (size_t i=out; i > 0; i--, form=form->tail)
    buf[i] = form->head;

  alist_write(&RlVm.compiler.stack, out, buf);
  return out;
}

static size_t compile_var(Symbol* name) {
  Chunk* code;
  Binding* bind;
  size_t out;

  code = RlVm.compiler.chunk;
  bind = lookup(code->envt, name);

  if (bind == NULL) // create binding, but raise an error if it isn't initialized by runtime
    bind = define(NULL, name, NOTHING, false);

  NsType type = get_ns_type(bind->ns);

  if (type == GLOBAL_NS)
    out = emit_instr(code, OP_LOADG, bind->offset);

  else if (type == PRIVATE_NS)
    out = emit_instr(code, OP_LOADP, bind->offset);

  else if (type == NONLOCAL_NS)
    out = emit_instr(code, OP_LOADS, bind->offset);

  else
    out = emit_instr(code, OP_LOADU, bind->offset);

  return out;
}

static size_t compile_val(Value val) {
  Chunk* code;
  size_t out, off;

  code = RlVm.compiler.chunk;

  if (val == NUL)
    out = emit_instr(code, OP_NUL);

  else if (val == TRUE)
    out = emit_instr(code, OP_TRUE);

  else if (val == FALSE)
    out = emit_instr(code, OP_FALSE);

  else if (val == ZERO)
    out = emit_instr(code, OP_ZERO);

  else if (val == ONE)
    out = emit_instr(code, OP_ONE);

  else if (val == tag(&EmptyBits))
    out = emit_instr(code, OP_EMPTY_BITS);

  else if (val == tag(&EmptyString))
    out = emit_instr(code, OP_EMPTY_STR);

  else if (val == tag(&EmptyList))
    out = emit_instr(code, OP_EMPTY_LIST);

  else if (val == tag(&EmptyTuple))
    out = emit_instr(code, OP_EMPTY_TUPLE);

  else if (val == tag(&EmptyVector))
    out = emit_instr(code, OP_EMPTY_VEC);

  else if (val == tag(&EmptyMap))
    out = emit_instr(code, OP_EMPTY_MAP);

  else {
    off = add_value(code, val);
    out = emit_instr(code, OP_LOADV, off);
  }

  return out;
}

static size_t compile_comb(List* form) {
  size_t out;
  Binding* macrob;
  Chunk* chunk;
  Function* macro;
  Value xpr;

  chunk = RlVm.compiler.chunk;

  if (is_special_form(form))
    out =as_sym(form->head)->special(form);

  else if ((macrob=is_macro_call(form))) {
    macro = as_func(macrob->value);
    xpr   = macro_expand(macro, chunk->envt, form);
    out   = compile_xpr(xpr);
  } else {
    
  }

  return out;
}

// special forms
size_t quote_form(List* form) {
  argco(2, form->arity, "quote");
  return compile_val(form->tail->head);
}

size_t do_form(List* form) {
  size_t arity, out;
  Value xpr;
  Chunk* chunk;

  chunk = RlVm.compiler.chunk;
  arity = vargco(2, form->arity, "do");

  if (arity == 2)
    out = compile_xpr(form->tail->head);

  else {
    arity = push_subxprs(form->tail);

    for (size_t i=0; i<arity; i++) {
      xpr = pop_subxpr();
      out = compile_xpr(xpr);

      if (i+1 < arity)
        emit_instr(chunk, OP_POP);
    }
  }

  return out;
}

size_t put_form(List* form) {
  Symbol* name;
  Binding* bind;
  Value var;
  Chunk* chunk;
  
  argco(3, form->arity, "put");
  chunk = RlVm.compiler.chunk;
  name  = as_sym_s(form->head, "put");
  var   = list_get(form, 2);
  bind  = lookup(chunk->envt, name);
}

size_t if_form(List* form) {
  size_t arity, offset1, offset2, offset3;
  Value  test, then, otherwise;
  Chunk* chunk;

  chunk = RlVm.compiler.chunk;

  /* consequent is otpional */
  argcos(2, form->arity, "if", 3, 4);

  arity     = push_subxprs(form->tail);
  test      = pop_subxpr();

  compile_xpr(test);

  offset1   = emit_instr(chunk, OP_JUMPF, 0);
  then      = pop_subxpr();

  compile_xpr(then);

  offset2   = emit_instr(chunk, OP_JUMP, 0);
  otherwise = arity == 3 ? NUL : pop_subxpr();

  offset3   = compile_xpr(otherwise);

  /* fill in jumps */
  chunk->code->data[offset1-1] = offset2-offset1;
  chunk->code->data[offset2-1] = offset3-offset2;

  return offset3;
}

// external API
Value macro_expand(Function* macro, Environment* envt, List* form) {
  size_t nsv, argc, sig_max;
  Value exp;
  Tuple* sig;
  Method* method;

  nsv     = save(3, tag(macro), tag(envt), tag(form));
  sig_max = get_sig_max(macro);
  sig     = get_macro_sig(sig_max, form->tail);
  method  = get_method(macro, sig);
  argc    = push_macro_args(envt, form);
  exp     = apply_cl(as_cls(method->fn), argc);

  unsave(nsv);

  return exp;
}

Closure* compile(void* name, CompilerState state, Value xpr);

static Value new_special_form(char* name, CompileFn special) {
  Symbol* sym;

  sym          = symbol(name);
  sym->special = special;

  return tag(sym);
}

void init_special_forms(void) {
  FunSym     = new_special_form("fun", fun_form);
  MacSym     = new_special_form("mac", mac_form);
  VarSym     = new_special_form("var", var_form);
  PutSym     = new_special_form("put", put_form);
  IfSym      = new_special_form("if",  if_form);
  WithSym    = new_special_form("with", with_form);
  QuoteSym   = new_special_form("quote", quote_form);
  DoSym      = new_special_form("do", do_form);
  UseSym     = new_special_form("use", use_form);
  PerformSym = new_special_form("perform", perform_form);
  HandleSym  = new_special_form("handle", handle_form);
}
