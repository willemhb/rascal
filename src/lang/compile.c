/**
 *
 * Toplevel compile API.
 *
 **/

// headers --------------------------------------------------------------------
#include "data/data.h"

#include "sys/error.h"
#include "sys/memory.h"

#include "lang/compile.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------
// compiler flags
enum {
  // syntax flags (indicates whether a particular form is restricted in the current context)
  CF_DEF      = 0x01, // allow def forms
  CF_PUT      = 0x02, // allow put forms

  // other compiler flags
  CF_TOPLEVEL = 0x80, // form is compiling at toplevel
};

// function prototypes --------------------------------------------------------
int  code_offset(Buf16* code);
int  emit_instr(Buf16* code, OpCode op, ...);
void fill_instr(Buf16* code, int offset, int val);
bool allow_def_form(flags_t flags);
bool allow_put_form(flags_t flags);
bool compiling_at_toplevel(flags_t flags);
flags_t forbid(flags_t flags, flags_t forbidden);
bool has_result(Expr x);

bool is_quote_form(List* form);
void compile_quote(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags);
bool is_def_form(List* form);
void compile_def(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags);
bool is_put_form(List* form);
void compile_put(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags);
bool is_if_form(List* form);
void compile_if(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags);
bool is_do_form(List* form);
void compile_do(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags);
bool is_fn_form(List* form);
void compile_fn(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags);
bool is_catch_form(List* form);
void compile_catch(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags);
bool is_throw_form(List* form);
void compile_throw(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags);

void compile_closure(Buf16* c_code, Env* vars, Alist* vals, Buf16* code, flags_t flags);
void compile_sequence(List* exprs, Env* vars, Alist* vals, Buf16* code, flags_t flags);
void compile_literal(Expr x, Env* vars, Alist* vals, Buf16* code, flags_t flags);
void compile_reference(Sym* s, Env* ref, Alist* vals, Buf16* code, flags_t flags);
void compile_funcall(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags);
void compile_expr(Expr x, Env* vars, Alist* vals, Buf16* code, flags_t flags);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
int code_offset(Buf16* code) {
  return code->binary.count;
}

int emit_instr(Buf16* code, OpCode op, ...) {
  // probably not very efficient, but easy to use
  instr_t buffer[3] = { op, 0, 0 };
  va_list va;
  va_start(va, op);
  int b = 1, n = op_arity(op);

  for ( int i=0; i < n; i++ ) {
    buffer[i+1]  = va_arg(va, int);
    b++;
  }

  va_end(va);

  buf16_write(code, buffer, b);

  return code_offset(code);
}

void fill_instr(Buf16* code, int offset, int val) {
  ((instr_t*)code->binary.vals)[offset] = val;
}

bool allow_def_form(flags_t flags) {
  return flags & CF_DEF;
}

bool allow_put_form(flags_t flags) {
  return flags & CF_PUT;
}

bool compiling_at_toplevel(flags_t flags) {
  return flags & CF_TOPLEVEL;
}

flags_t forbid(flags_t flags, flags_t forbidden) {
  return flags & ~forbidden;
}


bool has_result(Expr x) {
  bool out = true;

  if ( is_list(x) ) {
    List* lx = as_list(x);

    out = !(is_def_form(lx) || is_put_form(lx));
  }

  return out;
}

bool is_quote_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == QuoteStr;
}

void compile_quote(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  require_argco("quote", 1, form->count-1);

  Expr x = form->tail->head;

  compile_literal(x, vars, vals, code, flags);
}

bool is_def_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == DefStr;
}

void compile_def(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  require(allow_def_form(flags), "(def ...) not allowed in this context");
  require_argco("def", 2, form->count-1);

  Sym* n = as_sym_s("def", form->tail->head);

  require(!is_keyword(n), "can't assign to keyword %s", n->val->val);

  Ref* r = env_define(vars, n);

  compile_expr(form->tail->tail->head, vars, vals, code, flags);

  // internal defintions don't need a 'set' instruction because they don't need
  // to be removed from the stack
  if ( r->ref_type == REF_GLOBAL )
    emit_instr(code, OP_SET_GLOBAL, r->offset);
}

bool is_put_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == PutStr;
}

void compile_put(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  require(allow_put_form(flags), "(put ...) not allowed in this context");
  require_argco("put", 2, form->count-1);

  Sym* n = as_sym_s("put", form->tail->head);

  require(!is_keyword(n), "can't assign to keyword %s", n->val->val);

  Ref* r = env_resolve(vars, n, false);

  require(r != NULL, "can't assign to %s before it's defined", n->val->val);

  OpCode op; int i = r->offset;

  switch ( r->ref_type ) {
    case REF_GLOBAL:
      op = OP_SET_GLOBAL;
      break;

    case REF_CAPTURED_UPVAL:
      op = OP_SET_UPVAL;
      break;

    case REF_LOCAL:
      op = OP_SET_LOCAL;

      if ( i >= vars->arity ) // account for call frame
        i += FRAME_SIZE;

      break;

    default:
      unreachable();
  }

  compile_expr(form->tail->tail->head, vars, vals, code, flags);
  emit_instr(code, op, i);
}

bool is_if_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == IfStr;
}

void compile_if(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
    require_argco2("if", 2, 3, form->count-1);

    // don't allow assignments in conditionals
    flags     = forbid(flags, CF_DEF | CF_PUT);
    Expr test = form->tail->head;
    Expr then = form->tail->tail->head;
    Expr alt  = form->count == 3 ? NUL : form->tail->tail->tail->head;

    // compile different parts of the form, saving offsets to fill in later
    compile_expr(test, vars, vals, code, flags);
    emit_instr(code, OP_JUMP_F, 0);
    int off1 = code->binary.count;
    compile_expr(then, vars, vals, code, flags);
    emit_instr(code, OP_JUMP, 0);
    int off2 = code->binary.count;
    compile_expr(alt, vars, vals, code, flags);
    int off3 = code->binary.count;

    fill_instr(code, off1-1, off2-off1);
    fill_instr(code, off2-1, off3-off2);
}

bool is_do_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == DoStr;
}

void compile_do(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  require_vargco("do", 2, form->count-1);

  List* xprs = form->tail;

  compile_sequence(xprs, vars, vals, code, flags);
}

bool is_fn_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == FnStr;
}

void prepare_env(List* argl, Env* vars) {
  while ( argl->count > 0 ) {
    Expr x = argl->head;
    Sym* n = as_sym_s("fn", x);
    Ref* r = env_define(vars, n);
    int  o = r->offset;

    // ensure uniqueness
    require(o == vars->vars.count-1, "duplicate parameter name %s", n->val->val);
    vars->arity++;
    argl = argl->tail;
  }
}

void compile_fn(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  require_vargco("fn", 1, form->count-1);

  List* argl  = as_list_s("fn", form->tail->head);
  List* body  = form->tail->tail;
  Env*  lvars = mk_env(vars);

  preserve(3, tag_obj(lvars), NUL, NUL);
  prepare_env(argl, lvars);

  Alist* lvals = mk_alist(); add_to_preserved(1, tag_obj(lvals));
  Buf16* lcode = mk_buf16(); add_to_preserved(2, tag_obj(lcode));

  // flags to use for child forms
  flags_t lflags = CF_DEF | CF_PUT;

  while ( body->count > 0 ) {
    Expr hd = body->head;

    // allow internal definitions but only at the top of the fn body
    if ( allow_def_form(lflags) ) {
      if ( !is_list(hd) || !is_def_form(as_list(hd)) )
        lflags = forbid(lflags, CF_DEF);

        else {
          compile_def(as_list(hd), lvars, lvals, lcode, lflags);
          body = body->tail;
          continue;
        }
    }

    // compile remainder like body of a 'do' form
    compile_expr(hd, lvars, lvals, lcode, lflags);
    body = body->tail;

    if ( body->count > 0 && has_result(hd) )
      emit_instr(lcode, OP_POP);
  }

  int ncap = lvars->ncap, upvc = lvars->upvs.count;

  if ( ncap )
    emit_instr(lcode, OP_CAPTURE);

  emit_instr(lcode, OP_RETURN);

  // create chunk object
  Chunk* chunk = mk_chunk(lvars, lvals, lcode); add_to_preserved(0, tag_obj(chunk));
  Fun* fun     = mk_user_fun(chunk);

#ifdef RASCAL_DEBUG
  disassemble(fun);
#endif

  // add instructions in caller to load resulting function object
  compile_literal(tag_obj(fun), vars, vals, code, flags);

  // add instructions to capture upvalues in calling context
  if ( upvc > 0 ) {
    emit_instr(code, OP_CLOSURE, upvc);
    
    instr_t buffer[upvc*2];

    for ( int i=0, j=0; i < lvars->upvs.max_count && j < upvc; i++ ) {
      EMapKV* kv = &lvars->upvs.kvs[i];

      if ( kv->key == NULL )
        continue;

      Ref* r        = kv->val;
      buffer[j*2]   = r->ref_type == REF_LOCAL_UPVAL;
      buffer[j*2+1] = r->captures->offset;
      j++;
    }

    // write arguments to closure at once
    buf16_write(code, buffer, upvc*2);
  }
}

bool is_catch_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == CatchStr;
}

void compile_catch(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  require_vargco("catch", 2, form->count-1);
  
  List* handler = as_list_s( "catch", form->tail->head);

  require(is_fn_form(handler), "first argument to catch must be a handler");

  // compile the handler first so that it gets saved on the stack
  compile_fn(handler, vars, vals, code, flags);
  emit_instr(code, OP_CATCH);

  // add a jump instruction to skip the catch body if the catch handler is invoked
  int off0 = emit_instr(code, OP_JUMP, 0);

  // compile catch body like the body of a 'do' form (disallow definitions)
  compile_sequence(form->tail->tail, vars, vals, code, forbid(flags, CF_DEF));

  // discards context and handler and puts last expression of catch body at tos()
  int off1 = emit_instr(code, OP_ECATCH);

  fill_instr(code, off0-1, off1-off0);
}

bool is_throw_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == ThrowStr;
}

void compile_throw(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  require_argco("throw", 1, form->count-1);

  Expr arg = form->tail->head;

  compile_expr(arg, vars, vals, code, forbid(flags, CF_DEF | CF_PUT));
  emit_instr(code, OP_THROW);
}

void compile_sequence(List* xprs, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  while ( xprs->count > 0 ) {
    Expr x = xprs->head;
    compile_expr(x, vars, vals, code, flags);

    if ( xprs->count > 1 && has_result(x) )
      emit_instr(code, OP_POP);

    xprs = xprs->tail;
  }
}

void compile_literal(Expr x, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  (void)vars;
  (void)flags;

  int n = alist_push(vals, x);

  emit_instr(code, OP_GET_VALUE, n-1);
}

void compile_reference(Sym* s, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  (void)vals;
  (void)flags;

  Ref* r = env_resolve(vars, s, false);

  require(r != NULL, "undefined variable %s", s->val->val);

  OpCode op; int i = r->offset;

  switch ( r->ref_type ) {
    case REF_GLOBAL:         op = OP_GET_GLOBAL; break;
    case REF_LOCAL:          op = OP_GET_LOCAL;  break;
    case REF_CAPTURED_UPVAL: op = OP_GET_UPVAL;  break;
    default:                 unreachable();
  }

  emit_instr(code, op, i);
}

void compile_funcall(List* form, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  assert(form->count > 0);

  if ( is_quote_form(form) )
    compile_quote(form, vars, vals, code, flags);

  else if ( is_def_form(form) )
    compile_def(form, vars, vals, code, flags);

  else if ( is_put_form(form) )
    compile_put(form, vars, vals, code, flags);

  else if ( is_if_form(form) )
    compile_if(form, vars, vals, code, flags);

  else if ( is_do_form(form) )
    compile_do(form, vars, vals, code, flags);

  else if ( is_fn_form(form) )
    compile_fn(form, vars, vals, code, flags);

  else if ( is_catch_form(form) )
    compile_catch(form, vars, vals, code, flags);

  else if ( is_throw_form(form) )
    compile_throw(form, vars, vals, code, flags);

  else {
    int argc = form->count-1;
    Expr caller = form->head;
    flags = forbid(flags, CF_DEF | CF_PUT);
    form = form->tail;

    while ( form->count > 0 ) {
      Expr arg = form->head;
      compile_expr(arg, vars, vals, code, flags);
      form = form->tail;
    }

    // compile last so that caller is on top of the stack
    compile_expr(caller, vars, vals, code, flags);
    emit_instr(code, OP_CALL, argc);
  }
}

void compile_expr(Expr x, Env* vars, Alist* vals, Buf16* code, flags_t flags) {
  ExpType t = exp_type(x);
  
  if ( t == EXP_SYM ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) ) // keywords (symbols whose names begin with ':') are treated as literals
      compile_literal(x, vars, vals, code, flags);

    else
      compile_reference(s, vars, vals, code, flags);

  } else if ( t == EXP_LIST ) {
    List* l = as_list(x);

    if ( l->count == 0 ) // empty list is treated as a literal
      compile_literal(x, vars, vals, code, flags);

    else
      compile_funcall(l, vars, vals, code, flags);

  } else
    compile_literal(x, vars, vals, code, flags);
}

// external -------------------------------------------------------------------

Fun* toplevel_compile(List* form) {
  preserve(3, tag_obj(form), NUL, NUL);

  flags_t flags = CF_DEF | CF_PUT | CF_TOPLEVEL;

  Alist* vals  = mk_alist(); add_to_preserved(1, tag_obj(vals));
  Buf16* code = mk_buf16(); add_to_preserved(2, tag_obj(code));

  compile_funcall(form, &Globals, vals, code, flags);
  emit_instr(code, OP_RETURN);

  Chunk* chunk = mk_chunk(&Globals, vals, code); add_to_preserved(0, tag_obj(chunk)); // reuse saved slot
  Fun* out     = mk_user_fun(chunk);

#ifdef RASCAL_DEBUG
  disassemble(out);
#endif

  return out;
}

Fun* compile_file(List* forms) {
  preserve(3, tag_obj(forms), NUL, NUL);

  flags_t flags = CF_DEF | CF_PUT | CF_TOPLEVEL;

  Alist* vals  = mk_alist(); add_to_preserved(1, tag_obj(vals));
  Buf16* code = mk_buf16(); add_to_preserved(2, tag_obj(code));

  compile_sequence(forms, &Globals, vals, code, flags);
  emit_instr(code, OP_RETURN);

  Chunk* chunk = mk_chunk(&Globals, vals, code); add_to_preserved(0, tag_obj(chunk)); // reuse saved slot
  Fun* out     = mk_user_fun(chunk);

#ifdef RASCAL_DEBUG
  disassemble(out);
#endif

  return out;
}

// initialization -------------------------------------------------------------
