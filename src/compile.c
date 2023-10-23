#include "opcodes.h"

#include "runtime.h"

#include "stream.h"
#include "number.h"
#include "type.h"
#include "collection.h"
#include "environment.h"
#include "function.h"

#include "eval.h"
#include "compile.h"

// globals
Value DefSym, PutSym, LmbSym,
  DoSym, IfSym, QuoteSym,
  HandleSym, PerformSym, ResumeSym;

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

Chunk* new_chunk(void) {
  Chunk* out;

  out         = new_obj(&ChunkType, 0, 0); save(1, tag(out));
  out->vals   = new_alist(0);
  out->code   = new_binary16(0);

  return out;
}

// special forms
size_t def_form(List* form);
size_t put_form(List* form);
size_t lmb_form(List* form);
size_t do_form(List* form);
size_t quote_form(List* form);
size_t handle_form(List* form);
size_t perform_form(List* form);

// helpers
static CompFrame* push_compiler_frame(CompState state, Obj* name) {
  assert(RlVm.c.frame < &CompFrames[N_COMP]);
  CompFrame*   frame = RlVm.c.frame++;
  ScopeType    stype = state < COMPILER_FUNCTION ? GLOBAL_SCOPE : FUNCTION_SCOPE;
  Environment* penvt = frame > CompFrames && state > COMPILER_SCRIPT ? (frame-1)->envt : NULL;
  Handlers*    hndl  = frame > CompFrames ? (frame-1)->hndl : NULL;

  frame->state = state;
  frame->flags = 0;
  frame->name  = is_sym(name) ? as_sym(name)->name : as_str(name)->data;
  frame->code  = new_chunk();
  frame->envt  = new_envt(penvt, stype);
  frame->hndl  = state == COMPILER_HANDLE ? new_handlers(0, NULL, NULL, NULL) : hndl;

  set_annot(frame->code, NameOpt, tag(name));
  set_annot(frame->code, EnvtOpt, tag(frame->envt));

  return frame;
}

static Chunk* pop_compiler_frame(void) {
  assert(RlVm.c.frame > CompFrames);

  return (--RlVm.c.frame)->code;
}

static size_t comp_val(Value val);
static size_t comp_var(Symbol* name);
static size_t comp_comb(List* form);
static void   process_formals(List* formals, List* form);
static void   caputre_upvalues(NameSpace* ns);


static bool is_def_form(List* form) {
  return form->head == DefSym;
}

static bool is_lmb_form(List* form) {
  return form->head == LmbSym;
}

static bool is_literal(Value val) {
  if (is_sym(val))
    return !get_fl(as_sym(val), LITERAL);

  if (is_list(val))
    return as_list(val)->arity == 0;

  return true;
}

static size_t comp_xpr(Value xpr) {
  if (is_literal(xpr))
    return comp_val(xpr);

  else if (is_sym(xpr))
    return comp_var(as_sym(xpr));

  else
    return comp_comb(as_list(xpr));
}

static CompFrame* comp_frame(void) {
  return RlVm.c.frame;
}

static Alist* comp_stack(void) {
  return &RlVm.c.stack;
}

static flags_t comp_fl(void) {
  return RlVm.c.frame->flags;
}

static CompState comp_state(void) {
  return RlVm.c.frame->state;
}

static Handlers* comp_hndl(void) {
  return RlVm.c.frame->hndl;
}

static Environment* comp_envt(void) {
  return RlVm.c.frame->envt;
}

static Chunk* comp_code(void) {
  return RlVm.c.frame->code;
}

static Binary16* comp_instr(void) {
  return RlVm.c.frame->code->code;
}

static Alist* comp_vals(void) {
  return RlVm.c.frame->code->vals;
}

static char* comp_name(void) {
  return RlVm.c.frame->name;
}

static bool is_identifier(Value val) {
  return is_sym(val) && !get_fl(as_sym(val), LITERAL);
}

static bool is_keyword(Value val) {
  return is_sym(val) && get_fl(as_sym(val), LITERAL);
}

static bool is_special_form(List* form) {
  return is_sym(form->head) && as_sym(form->head)->special != NULL;
}

static Function* is_macro_call(List* form) {
  Function* out = NULL;

  if (is_sym(form->head))
    out = lookup_syntax(comp_envt(), as_sym(form->head));

  return out;
}

static size_t emit_instr(Chunk* code, OpCode op, ...) {
  uint16_t buf[3] = { op, 0, 0 };
  size_t argc = opcode_argc(op);
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

static Value* peek_subxpr(int i) {
  Alist* stk = comp_stack();
  
  if (i < 0)
    i += stk->cnt;

  assert(i >= 0 && i < (int)stk->cnt);
  return &stk->data[i];
}

static size_t push_subxpr(Value x) {
  return alist_push(&RlVm.c.stack, x);
}

static Value pop_subxpr(void) {
  return alist_pop(&RlVm.c.stack);
}

static size_t push_subxprs(List* form) {
  size_t out = form->arity;
  Value buf[out];

  for (size_t i=out; i > 0; i--, form=form->tail)
    buf[i] = form->head;

  alist_write(&RlVm.c.stack, out, buf);
  return out;
}

static size_t comp_var(Symbol* name) {
  CompFrame* frame;
  Chunk* code;
  Binding* bind;
  size_t out;
  
  frame = RlVm.c.frame;
  code  = frame->code;
  bind  = lookup(frame->envt, name, true);

  require(bind != NULL, comp_name(), "unbound symbol `%s`", name);

  NsType type = get_ns_type(bind->ns);

  if (type == GLOBAL_NS)
    out = emit_instr(code, OP_GETG, bind->offset);

  else if (type == NONLOCAL_NS)
    out = emit_instr(code, OP_GETU, bind->offset);

  else
    out = emit_instr(code, OP_GETL, bind->offset);

  return out;
}

static size_t comp_val(Value val) {
  CompFrame* frame;
  Chunk* code;
  size_t out, off;

  frame = RlVm.c.frame;
  code  = frame->code;

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

  else if (is_small(val) && fits(val, INT16_MIN, INT16_MAX))
    out = emit_instr(code, OP_GETI16, as_small(val));

  else if (is_glyph(val) && fits(val, 0, INT16_MAX))
    out = emit_instr(code, OP_GETG16, as_glyph(val));
      
  else {
    off = add_value(code, val);
    out = emit_instr(code, OP_GETV, off);
  }

  return out;
}

static size_t comp_comb(List* form) {
  size_t out;
  CompFrame* frame;
  Function* macro;
  Value xpr;

  frame = RlVm.c.frame;

  if (is_special_form(form))
    out =as_sym(form->head)->special(form);

  else if ((macro=is_macro_call(form))) {
    xpr   = macro_expand(macro, frame->envt, form);
    out   = comp_xpr(xpr);
  } else {
    
  }

  return out;
}


static size_t comp_defun(Symbol* name, List* lmb) {
  Binding* bind = defun(comp_envt(), name, FINAL);
  NsType nst = get_ns_type(bind->ns);
  size_t out = 0;

  push_compiler_frame(COMPILER_DEFUN, (Obj*)name);
  lmb_form(lmb);

  if (nst == GLOBAL_NS)
    out = emit_instr(comp_code(), OP_SPECG, bind->offset);

  else
    out = emit_instr(comp_code(), OP_SPECL, bind->offset);

  return out;
}

static size_t comp_defmac(Symbol* name, List* lmb) {
  Environment* envt = comp_envt();

  require(get_scope_type(envt) == GLOBAL_SCOPE,
          "def",
          "local macro definitions are not supported.");
  
  Binding* bind     = defmac(comp_envt(), name, FINAL);

  push_compiler_frame(COMPILER_DEFMAC, (Obj*)name);
  lmb_form(lmb);

  Chunk* code  = pop_compiler_frame();
  Tuple* sig   = as_tuple(get_annot(code, SignatureOpt));
  bool va      = get_annot(code, VaOpt) == TRUE;
  Function* fn = as_func(bind->value);

  add_method(fn, sig, (Obj*)code, MACRO|va*VARIADIC);

  return comp_instr()->cnt;
}

static size_t comp_seq(List* body) {
  size_t arity;
  size_t out;
  Value xpr;
  
  
  if (body->arity == 1)
    out = comp_xpr(body->head);

  else {
    arity = push_subxprs(body);
    
    for (size_t i=0; i<arity; i++) {
      xpr = pop_subxpr();
      out = comp_xpr(xpr);
      
      if (i+1 < arity)
        emit_instr(comp_code(), OP_POP);
    }
  }
  
  return out;
}

// special forms
size_t quote_form(List* form) {
  argco(2, form->arity, "quote");
  return comp_val(form->tail->head);
}

size_t do_form(List* form) {
  vargco(2, form->arity, "do");
  
  return comp_seq(form->tail);
}

size_t def_form(List* form) {
  Symbol* name;
  Binding* bind;
  Value val;
  size_t out;
  NsType nst;

  argco(3, form->arity, "def");
  save(1, tag(form));

  name = as_sym_s(form->tail->head, "def");
  val  = list_get(form, 2);
  bind = lookup(comp_envt(), name, false);

  if (is_list(val) && is_lmb_form(as_list(val))) {
      List* lmb = as_list(val);

      if (get_annot(lmb, MacroOpt) == TRUE)
        return comp_defmac(name, lmb);

      else
        return comp_defun(name, lmb);
  }

  require(!is_bound(comp_envt(), name), "def",
          "redefinition of bound symbol `%s`",
          name->name);

  comp_xpr(val);
  
  nst = get_ns_type(bind->ns);
  out = emit_instr(comp_code(), nst == GLOBAL_NS ? OP_PUTG : OP_PUTL, bind->offset);

  return out;
}

static void process_formals(List* form, List* formals) {
  size_t n_args, n_annot = 0;

  bool va = false;
  Tuple* sig;
  Environment* envt = comp_envt();

  for (; formals->arity > 0; formals=formals->tail) {
    Value formal = formals->head;
    Symbol* var;
    Type* type;

    if (is_list(formal)) {
      if (va)
        syntax_error(form, "typed variadic arguments are not supported");

      List* tf = as_list(formal);

      Value   t_xpr = tf->head;
      var   = as_sym_s(tf->tail->head, "lmb");

      if (!is_type(t_xpr)) {
        Symbol* t_name  = as_sym_s(t_xpr, "lmb");
        Binding* t_bind = lookup(comp_envt(), t_name, false);

        if (t_bind == NULL)
          syntax_error(form, "`%s` does not name a type", t_name->name);

        else if (get_ns_type(t_bind->ns) != GLOBAL_NS)
          syntax_error(form, "could not resolve type for `%s`", t_name->name);

        else if (!is_type(t_bind->value))
          syntax_error(form, "`%s` does not name a type", t_name->name);

        else
          type = as_type(t_bind->value);
      } else {
        type = as_type(t_xpr);
      }
    } else if (is_sym(formal)) {
      if (formal == AmpSym) {
        if (formals->arity != 2)
          syntax_error(form, "extra arguments after `&`");

        va = true;
        continue;
      } else {
        var  = as_sym(formal);
        type = &AnyType;
      }
    } else {
      syntax_error(form, "formal argument is not an identifier");
    }

    if (is_bound(envt, var))
      syntax_error(form, "formal argument appears twice");

    n_args++;

    if (!va) {
      n_annot++;
      push_subxpr(tag(type));
    }

    define(envt, var, NUL, 0);
  }

  /* compute and save signature */ 
  sig = new_tuple(n_annot, peek_subxpr(-n_annot));

  set_annot(comp_code(), SignatureOpt, tag(sig));
  set_annot(comp_code(), VaOpt, tag(va));
}

static void capture_upvalues(NameSpace* ns) {
  Chunk*       chunk = comp_code();
  Binary16*    code  = chunk->code;

  emit_instr(chunk, OP_CLOSURE, ns->kv_cnt);

  for (size_t i=0; i<ns->kv_cnt; i++) {
    Binding* bind = ns->kvs[i].val;

    binary16_pushn(code, 2, is_local_upval(bind), bind->offset);
  }
}

size_t lmb_form(List* form) {
  vargco(3, form->arity, "lmb");

  NameSpace* cl_upvals;
  Chunk* ca_code, * cl_code;
  CompState state;
  List* formals, * body;
  size_t out, off;

  state   = comp_state();
  formals = as_list_s(form->tail->head, "lmb");
  body    = form->tail->tail;

  if (state == COMPILER_DEFMAC) {
    merge_annot(comp_code(), form);
    process_formals(form, formals);
    comp_seq(body);
    
    ca_code = pop_compiler_frame();

    out = 0;
  } else if (state == COMPILER_DEFUN) {
    merge_annot(comp_code(), form);
    process_formals(form, formals);
    comp_seq(body);

    cl_upvals = comp_envt()->upvals;
    cl_code   = pop_compiler_frame();
    ca_code   = comp_code();
    off       = add_value(ca_code, tag(cl_code));

    emit_instr(ca_code, OP_GETV, off);
    capture_upvalues(cl_upvals);

    // insert 
    
  } else {
    process_formals(form, formals);
    comp_seq(body);

    cl_code = pop_compiler_frame();
    ca_code = comp_code();
    off     = add_value(ca_code, tag(cl_code));
    
  }

  return out;
}

size_t put_form(List* form) {
  Symbol* name;
  Binding* bind;
  Value var;
  Chunk* chunk;
  NsType nst;
  size_t out;

  argco(3, form->arity, "put");
  save(1, tag(form));

  name  = as_sym_s(form->head, "put");
  var   = list_get(form, 2);
  bind  = lookup(comp_envt(), name, true);

  require(bind != NULL, comp_name(), "unbound symbol `%s`", name->name);
  require(!get_fl(bind, FINAL), comp_name(), "can't rebind final symbol `%s`", name->name);
  comp_xpr(var);

  nst   = get_ns_type(bind->ns);
  chunk = comp_code();

  if (nst == GLOBAL_NS)
    out = emit_instr(chunk, OP_PUTG);

  else if (nst == LOCAL_NS)
    out = emit_instr(chunk, OP_PUTL);

  else
    out = emit_instr(chunk, OP_PUTU);

  return out;
}

size_t if_form(List* form) {
  size_t arity, offset1, offset2, offset3;
  Value  test, then, otherwise;
  Chunk* chunk;

  chunk = comp_code();

  /* consequent is otpional */
  argcos(2, form->arity, "if", 3, 4);

  arity     = push_subxprs(form->tail);
  test      = pop_subxpr();

  comp_xpr(test);

  offset1   = emit_instr(chunk, OP_JUMPF, 0);
  then      = pop_subxpr();

  comp_xpr(then);

  offset2   = emit_instr(chunk, OP_JUMP, 0);
  otherwise = arity == 3 ? NUL : pop_subxpr();

  offset3   = comp_xpr(otherwise);

  /* fill in jumps */
  chunk->code->data[offset1-1] = offset2-offset1;
  chunk->code->data[offset2-1] = offset3-offset2;

  return offset3;
}

// external API
static Value new_special_form(char* name, CompileFn special) {
  Symbol* sym;

  sym          = symbol(name);
  sym->special = special;

  return tag(sym);
}

void init_syntax(void) {
  DefSym     = new_special_form("def", def_form);
  PutSym     = new_special_form("mac", put_form);
  LmbSym     = new_special_form("var", lmb_form);
  DoSym      = new_special_form("do", do_form);
  IfSym      = new_special_form("if",  if_form);
  QuoteSym   = new_special_form("quote", quote_form);
  HandleSym  = new_special_form("do", handle_form);
  PerformSym = new_special_form("perform", perform_form);

  // other syntax
  AmpSym     = tag(symbol("&"));
  ResumeSym  = tag(symbol("resume"));
}
