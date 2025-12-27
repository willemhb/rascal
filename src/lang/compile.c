#include <string.h>

#include "util/util.h"
#include "lang/compile.h"
#include "lang/exec.h"
#include "lang/read.h"
#include "vm.h"
#include "val.h"

// Globals --------------------------------------------------------------------
// compiler flags
enum {
  CF_NO_POP  = 0x01,
  CF_ALLOW_DEF = 0x02,
  CF_TAIL_POS = 0x04,
};

// Prototypes -----------------------------------------------------------------
// Helpers --------------------------------------------------------------------
void* syntax_as_obj_s(RlState* rls, List* form, Type* e, Expr x);
Sym* syntax_as_sym_s(RlState* rls, List* form, Expr x);
List* syntax_as_list_s(RlState* rls, List* form, Expr x);
Fun* syntax_as_fun_s(RlState* rls, List* form, Expr x);

void emit_instr(RlState* rls, Chunk* code, OpCode op, ...);
void fill_instr(RlState* rls, Chunk* code, int offset, int val);
int add_chunk_constant(RlState* rls, Chunk* code, Expr c);
bool is_special_form(List* form, char* form_name);
bool is_syntax(Expr x, char* stx_kw);
void prepare_env(RlState* rls, List* form, Chunk* code, int* argc, bool* va);
Ref* is_macro_call(RlState* rls, List* form, Env* vars);
Expr get_macro_expansion(RlState* rls, List* form, Env* vars, Ref* macro_ref);
Sym* get_assignment(RlState* rls, List* form);

// Dispatch -------------------------------------------------------------------
void compile_quote(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_def(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_def_stx(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_def_multi(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_def_method(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_put(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_if(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_and(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_or(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_do(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_control(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_raise(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_fn(RlState* rls, List* form, Chunk* code, int* flags, int* line);

void compile_closure(RlState* rls, List* form, Chunk* pcode, Fun* fun, int* flags, int* line);
void compile_sequence(RlState* rls, List* exprs, Chunk* code, int* flags, int* line);
void compile_literal(RlState* rls, Expr x, Chunk* code, int* flags, int* line);
void compile_reference(RlState* rls, Sym* s, Chunk* code, int* flags, int* line);
void compile_funcall(RlState* rls, List* form, Chunk* code, int* flags, int* line);
void compile_expr(RlState* rls, Expr x, Chunk* code, int* flags, int* line);

// Implementations ------------------------------------------------------------
// Helpers --------------------------------------------------------------------
void* syntax_as_obj_s(RlState* rls, List* form, Type* e, Expr x) {
  syntax_require_argtype(rls, form, e, x);
  return as_obj(x);
}

Sym* syntax_as_sym_s(RlState* rls, List* form, Expr x) {
  return syntax_as_obj_s(rls, form, &SymType, x);
}

List* syntax_as_list_s(RlState* rls, List* form, Expr x) {
  return syntax_as_obj_s(rls, form, &ListType, x);
}

Fun* syntax_as_fun_s(RlState* rls, List* form, Expr x) {
  return syntax_as_obj_s(rls, form, &FunType, x);
}

void emit_instr(RlState* rls, Chunk* chunk, OpCode op, ...) {
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

  code_buf_write(rls, &chunk->code, buffer, b);
}

void fill_instr(RlState* rls, Chunk* code, int offset, int val) {
  (void)rls;

  chunk_code(code)[offset] = val;
}

int add_chunk_constant(RlState* rls, Chunk* code, Expr c) {
  exprs_push(rls, &code->vals, c);

  return chunk_valsc(code) - 1;
}


bool is_special_form(List* form, char* f_name) {
  Expr hd = form->head;

  return is_sym(hd) && sym_val_eql(as_sym(hd), f_name);
}

bool is_syntax(Expr x, char* stx_kw) {
  return is_sym(x) && sym_val_eql(as_sym(x), stx_kw);
}

void prepare_env(RlState* rls, List* form, Chunk* code, int* argc, bool* va) {
  *va = false;
  *argc = 0;

  Env* vars = code->vars;
  List* argl = syntax_as_list_s(rls, form, list_snd(form));

  while ( argl->count > 0 ) {
    Expr x = argl->head;
    Sym* n = syntax_as_sym_s(rls, form, x);

    // check for & rest parameter marker
    if ( sym_val_eql(n, "&") ) {
      argl = argl->tail;
      syntax_require(rls, argl->count == 1, form,
                     "& not followed by exactly one parameter");
      x = argl->head;
      n = syntax_as_sym_s(rls, form, x);
      *va = true;
    }

    bool a;
    env_define(rls, vars, n, false, false, &a);

    // ensure uniqueness
    syntax_require(rls, a, form, "duplicate parameter name %s", sym_val(n));

    if ( !(*va) )
      (*argc)++;

    argl = argl->tail;
  }
}

// Check if a form is a macro call and return the macro Ref if so
Ref* is_macro_call(RlState* rls, List* form, Env* vars) {
  if (form->count == 0)
    return NULL;

  Expr head = form->head;
  if (!is_sym(head))
    return NULL;

  Sym* s = as_sym(head);
  Ref* r = env_resolve(rls, vars, s, false);

  if (r != NULL && r->macro)
    return r;

  return NULL;
}

// Expand a macro call at compile time
Expr get_macro_expansion(RlState* rls, List* form, Env* vars, Ref* macro_ref) {
  StackRef top = rls->s_top;
  // prepare call state on stack
  // push function and implicit arguments (expanding form and environment)
  stack_check_limit(rls, 3 + form->tail->count);
  stack_preserve(rls, 3, macro_ref->val, tag_obj(form), tag_obj(vars));

  // The macro receives: whole form, environment, then individual arguments (all unevaluated)
  List* args = form->tail;
  int argc = 2 + push_list(rls, args);
  Expr result = exec_code(rls, argc, 0);
  rls->s_top = top;

  return result;
}

Sym* get_assignment(RlState* rls, List* form) {
  Sym* n = syntax_as_sym_s(rls, form, list_snd(form));
  syntax_require(rls, !is_keyword(n), form,
                 "can't assign to keyword %s", sym_val(n));

  return n;
}

// Dispatch -------------------------------------------------------------------
void compile_quote(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  syntax_require_argco(rls, form, 1);
  Expr x = form->tail->head;
  compile_literal(rls, x, code, flags, line);
}

void compile_def(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  int candef = *flags & CF_ALLOW_DEF;
  syntax_require(rls, candef, form, "`def` not allowed in current context.");
  syntax_require_argco(rls, form, 2);
  bool a;
  Sym* n = get_assignment(rls, form);
  Ref* r = env_define(rls, code->vars, n, false, false, &a);
  syntax_require(rls, a, form, "%s already bound in this environment", sym_val(n));
  int i  = r->offset;
  OpCode op;

  switch ( r->ref_type ) {
    case REF_GLOBAL:
      op = OP_SET_GLOBAL;
      break;

    case REF_LOCAL:
      op = OP_SET_LOCAL;
      *flags |= CF_NO_POP; // signal no pop (so that definition stays on the stack)
      break;

    default:
      unreachable();
  }

  *flags &= ~CF_ALLOW_DEF; // temporarily disable to avoid weird shit with nested definitions
  Expr x = list_thd(form);

  if ( is_list(x) && is_special_form(as_list(x), "fn") ) {
    List* fn_form = as_list(x);
    Fun* def_fun = mk_fun_s(rls, n, false, false);
    compile_closure(rls, fn_form, code, def_fun, flags, line);
    stack_pop(rls);
  } else {
    compile_expr(rls, x, code, flags, line);
  }
  emit_instr(rls, code, op, i);
  *flags |= CF_ALLOW_DEF;
}

void compile_def_stx(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  Env* vars = code->vars;
  syntax_require(rls, is_global_env(vars), form, "local macros not supported.");
  syntax_require_argco(rls, form, 2);
  Sym* n = get_assignment(rls, form);
  List* fn_form = syntax_as_list_s(rls, form, list_thd(form));
  syntax_require(rls, is_special_form(fn_form, "fn"), form, "syntax transformer is not a `fn` form.");
  // Define the macro in the environment and mark it as a macro (ensure the binding doesn't already exist)
  bool a;
  Ref* r = env_define(rls, vars, n, true, true, &a);
  syntax_require(rls, a, form, "%s already bound.", sym_val(n));
  // create function to hold method and assign to ref immediately (not at run time)
  Fun* macro_fun = mk_fun(rls, n, true, true);
  r->val = tag_obj(macro_fun);
  // compile_closure handles adding the method based on what kind of fun it's passed
  compile_closure(rls, fn_form, code, macro_fun, flags, line);

  *flags |= CF_NO_POP; // signal don't emit a pop instruction
}

void compile_def_multi(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  Env* vars = code->vars;
  syntax_require_argco(rls, form, 2);
  syntax_require(rls, is_global_env(vars), form, "local generics not supported." );
  Sym* n = get_assignment(rls, form);
  List* fn_form = syntax_as_list_s(rls, form, list_thd(form));
  syntax_require(rls, is_special_form(fn_form, "fn"), form, "method is not a `fn` form.");
  // Define the function in the environment and mark it as a generic (ensure the binding doesn't already exist)
  bool a;
  Ref* r = env_define(rls, vars, n, false, true, &a);
  syntax_require(rls, a, form, "%s already bound.", sym_val(n));

  // create function to hold method and assign to ref immediately (not at run time)
  Fun* generic_fun = mk_fun(rls, n, false, true);
  r->val = tag_obj(generic_fun);

  // compile_closure handles adding the method based on what kind of fun it's passed
  compile_closure(rls, fn_form, code, generic_fun, flags, line);
}

void compile_def_method(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  Env* vars; Sym* n; List* fn_form; Ref* r; Fun* g_fun;

  vars = code->vars;
  syntax_require_argco(rls, form, 2);
  syntax_require(rls, is_global_env(vars), form, "local generics not supported.");
  n = get_assignment(rls, form);
  fn_form = syntax_as_list_s(rls, form, list_thd(form));
  syntax_require(rls, is_special_form(fn_form, "fn"), form, "method is not a `fn` form.");
  // Resolve the function binding (ensure it exists)
  r = env_resolve(rls, vars, n, false);
  syntax_require(rls, r != NULL, form, "%s not bound to generic", sym_val(n));
  g_fun = syntax_as_fun_s(rls, form, r->val);
  syntax_require(rls, g_fun->generic, form, "%s does not support overloads.", sym_val(n));
  // compile_closure handles adding the method based on what kind of fun it's passed
  compile_closure(rls, fn_form, code, g_fun, flags, line);
}

void compile_put(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  Sym* n; Ref* r; OpCode op; int i; Env* vars;

  syntax_require_argco(rls, form, 2);
  vars = code->vars;
  n = get_assignment(rls, form);
  r = env_resolve(rls, vars, n, false);
  syntax_require(rls, r != NULL, form, "can't assign to %s before it's defined.", sym_val(n));
  i = r->offset;

  switch ( r->ref_type ) {
    case REF_GLOBAL:
      op = OP_SET_GLOBAL;
      break;

    case REF_CAPTURED_UPVAL:
      op = OP_SET_UPVAL;
      break;

    case REF_LOCAL:
      op = OP_SET_LOCAL;
      break;

    default:
      unreachable();
  }

  compile_expr(rls, list_thd(form), code, flags, line);
  emit_instr(rls, code, op, i);
}

void compile_if(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  syntax_require_vargco(rls, form, 2);
  List* exprs = form->tail;
  int exprc = exprs->count;
  int testc = exprc / 2;
  // keep track of offsets that need to be filled in once the total code size is known
  int then_offsets[testc], off1, off2, end;
  Expr last_expr = NUL;

  for ( int i=0; exprs->count > 0; i++, exprs = exprs->tail->tail ) {
    Expr test = list_fst(exprs);
    Expr then;
    
    if ( is_syntax(test, "otherwise") ) {
      syntax_require(rls, exprs->count == 2, form, "malformed final alternative.");
      testc--;
      last_expr = list_snd(exprs);
      break;
    }
    
    if ( exprs->count == 1 ) {
      last_expr = list_fst(exprs);
      break;
    } else {
      then = list_snd(exprs);
      compile_expr(rls, test, code, flags, line);
      emit_instr(rls, code, OP_JUMP_F, 0);
      off1 = chunk_codec(code);
      compile_expr(rls, then, code, flags, line);
      emit_instr(rls, code, OP_JUMP, 0);
      off2 = chunk_codec(code);
      fill_instr(rls, code, off1-1, off2-off1);
      then_offsets[i] = off2; // save to fill in once full code size is known
    }
  }
  
  compile_expr(rls, last_expr, code, flags, line);
  end = chunk_codec(code);

  // fill in missing jump offsets
  for ( int i=0; i<testc; i++ ) {
    int off = then_offsets[i];
    fill_instr(rls, code, off-1, end-off);
  }
}

void compile_and(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  List* exprs = form->tail;
  int exprc = exprs->count;

  if ( exprc == 0 )
    emit_instr(rls, code, OP_TRUE);

  else {
    // keep track of offsets that need to be filled in once the total code size is known
    int offsets[exprc-1];

    for ( int i=0; i<exprc-1; i++ ) {
      compile_expr(rls, exprs->head, code, flags, line);
      emit_instr(rls, code, OP_PJUMP_F, 0);
      offsets[i] = chunk_codec(code);
      exprs = exprs->tail;
    }

    compile_expr(rls, list_fst(exprs), code, flags, line);

    int end = chunk_codec(code);

    for ( int i=0; i<exprc-1; i++ ) {
      int offset = offsets[i];

      fill_instr(rls, code, offset-1, end-offset);
    }
  }
}

void compile_or(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  List* exprs = form->tail;
  int exprc = exprs->count;

  if ( exprc == 0 )
    emit_instr(rls, code, OP_FALSE);

  else {
    // keep track of offsets that need to be filled in once the total code size is known
    int offsets[exprc-1];

    for ( int i=0; i<exprc-1; i++ ) {
      compile_expr(rls, exprs->head, code, flags, line);
      emit_instr(rls, code, OP_PJUMP_T, 0);
      offsets[i] = chunk_codec(code);
      exprs = exprs->tail;
    }

    compile_expr(rls, list_fst(exprs), code, flags, line);

    int end = chunk_codec(code);

    for ( int i=0; i<exprc-1; i++ ) {
      int offset = offsets[i];

      fill_instr(rls, code, offset-1, end-offset);
    }
  }
}

void compile_do(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  syntax_require_vargco(rls, form, 1);
  compile_sequence(rls, form->tail, code, flags, line);
}

void compile_control(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  StackRef top = rls->s_top;
  syntax_require_argco(rls, form, 2);
  List* body_form = syntax_as_list_s(rls, form, list_snd(form));
  List* catch_form = syntax_as_list_s(rls, form, list_thd(form));
  syntax_require(rls, is_special_form(body_form, "fn"), body_form,
                 "control body must be a `fn` form.");
  syntax_require(rls, is_special_form(catch_form, "fn"), catch_form,
                 "control handler must be a `fn` form.");
  Sym* catch_name = mk_sym_s(rls, "<control-handler>");
  Fun* catch_fun = mk_fun_s(rls, catch_name, false, false);
  Sym* body_name = mk_sym_s(rls, "<control-body>");
  Fun* body_fun = mk_fun_s(rls, body_name, false, false);

  // compile both closures so that the final result is catch and body on stack
  // in that order
  compile_closure(rls, catch_form, code, catch_fun, flags, line);
  compile_closure(rls, body_form, code, body_fun, flags, line);

  // emit CATCH/ECATCH instructions
  emit_instr(rls, code, OP_CATCH);
  emit_instr(rls, code, OP_ECATCH);
  rls->s_top = top;
}

void compile_raise(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  syntax_require_argco(rls, form, 1);
  compile_expr(rls, list_snd(form), code, flags, line);
  emit_instr(rls, code, OP_RAISE);
}

void compile_fn(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  syntax_require_vargco(rls, form, 2);
  StackRef top = rls->s_top;
  Sym* n = mk_sym_s(rls, "fn");
  Fun* cl_fun = mk_fun_s(rls, n, false, false);
  compile_closure(rls, form, code, cl_fun, flags, line);
  rls->s_top = top;
}

void compile_closure(RlState* rls, List* form, Chunk* code, Fun* fun, int* flags, int* line) {
  syntax_require_vargco(rls, form, 2);

  bool va;
  int argc, upvc, lflags = CF_ALLOW_DEF, linec  = line ? 1 : form->line,
    *linebuf = line ? &linec : NULL;
  StackRef top = rls->s_top;
  Chunk* lcode = mk_chunk_s(rls, code->vars, fun->name, code->file);
  List* body = form->tail->tail;

  // validate arguments and prepare environment
  prepare_env(rls, form, lcode, &argc, &va);

  // compile definitions at the top of the function body
  for (; body->count > 0 &&
         is_list(body->head) &&
         is_special_form(as_list(body->head), "def"); body=body->tail )
      compile_def(rls, as_list(body->head), lcode, &lflags, line);

  // definitions only allowed at the top of a function body
  lflags &= ~CF_ALLOW_DEF;

  compile_sequence(rls, body, lcode, &lflags, linebuf);
  finalize_chunk(rls, lcode, linec);

  Method* method = mk_user_method_s(rls, fun, argc, va, lcode);

  // what to do with the compiled closure depends on the flags of fun
  if ( fun->macro ) { // macros should be evaluated immediately
    fun_add_method(rls, fun, method);
  }

  else { // closure must be handled at runtime, store method in constants
    compile_literal(rls, tag_obj(method), code, flags, line);

    upvc = method_upvc(method);

    if ( upvc > 0 ) {
      emit_instr(rls, code, OP_CLOSURE, upvc);

      instr_t buffer[upvc*2];

      for ( int i=0, j=0; i < env_upval_maxc(lcode->vars) && j < upvc; i++ ) {
        EMapKV* kv = &env_upval_refs(lcode->vars)[i];
        
        if ( kv->key == NULL )
          continue;
        
        Ref* r        = kv->val;
        buffer[j*2]   = r->ref_type == REF_LOCAL_UPVAL;
        buffer[j*2+1] = r->captures->offset;
        j++;
      }

      // write arguments to closure at once
      code_buf_write(rls, &lcode->code, buffer, upvc*2);
    }

    // this handles binding the method to its corresponding function object
    emit_instr(rls, code, OP_ADD_METHOD);
  }

  rls->s_top = top;
}

void compile_sequence(RlState* rls, List* xprs, Chunk* code, int* flags, int* line) {
  while ( xprs->count > 0 ) {
    *flags &= ~CF_NO_POP; // reset NO_POP flag if necessary
    Expr x = xprs->head;
    compile_expr(rls, x, code, flags, line);

    if ( xprs->count > 1 && (*flags & CF_NO_POP) != CF_NO_POP )
      emit_instr(rls, code, OP_POP);

    xprs = xprs->tail;
  }
}

void compile_literal(RlState* rls, Expr x, Chunk* code, int* flags, int* line) {
  (void)line;
  (void)flags;

  if ( is_num(x) ) {
    if ( x == RL_ZERO )
      emit_instr(rls, code, OP_ZERO);

    else if ( x == RL_ONE )
      emit_instr(rls, code, OP_ONE);

    else {
      Num n = as_num(x);

      if ( is_int(n) && n <= INT16_MAX && n >= INT16_MIN )
        emit_instr(rls, code, OP_SMALL, (short)n);

      else
        goto fallback;
    }
  } else if ( is_glyph(x) ) {
    if ( x == EOS )
      emit_instr(rls, code, OP_EOS);

    else
      emit_instr(rls, code, OP_GLYPH, as_glyph(x));
  } else if ( x == TRUE ) {
    emit_instr(rls, code, OP_TRUE);
  } else if ( x == FALSE ) {
    emit_instr(rls, code, OP_FALSE);
  } else if ( x == NUL ) {
    emit_instr(rls, code, OP_NUL);
  } else {
  fallback:
    int n = add_chunk_constant(rls, code, x);

    emit_instr(rls, code, OP_GET_VALUE, n);
  }
}

void compile_reference(RlState* rls, Sym* s, Chunk* code, int* flags, int* line) {
  (void)line;
  (void)flags;

  Ref* r = env_resolve(rls, code->vars, s, false);
  require(rls, r != NULL, "undefined variable %s", sym_val(s));
  OpCode op; int i = r->offset;

  switch ( r->ref_type ) {
    case REF_GLOBAL:
      op = OP_GET_GLOBAL;
      break;

    case REF_LOCAL:
      op = OP_GET_LOCAL;
      break;

    case REF_CAPTURED_UPVAL:
      op = OP_GET_UPVAL;
      break;

    default:
      unreachable();
  }

  emit_instr(rls, code, op, i);
}

void compile_funcall(RlState* rls, List* form, Chunk* code, int* flags, int* line) {
  assert(form->count > 0);

  if ( line != NULL && form->line > *line ) {
    add_line_number(rls, code, *line);
    *line = form->line;
  }

  Env* vars = code->vars;
  // Check for macro calls first - macros are expanded before other processing
  Ref* macro_ref = is_macro_call(rls, form, vars);

  if (macro_ref != NULL) {
    Expr expanded = get_macro_expansion(rls, form, vars, macro_ref);
    stack_push(rls, expanded); // save expanded expression
    compile_expr(rls, expanded, code, flags, line);
    stack_pop(rls); // unsave expanded expression
  }

  else if ( is_special_form(form, "quote") )
    compile_quote(rls, form, code, flags, line);

  else if ( is_special_form(form, "def") )
    compile_def(rls, form, code, flags, line);

  else if ( is_special_form(form, "def-stx") )
    compile_def_stx(rls, form, code, flags, line);

  else if ( is_special_form(form, "def-multi") )
    compile_def_multi(rls, form, code, flags, line);

  else if ( is_special_form(form, "def-method") )
    compile_def_method(rls, form, code, flags, line);

  else if ( is_special_form(form, "put") )
    compile_put(rls, form, code, flags, line);

  else if ( is_special_form(form, "if") )
    compile_if(rls, form, code, flags, line);

  else if ( is_special_form(form, "and") )
    compile_and(rls, form, code, flags, line);

  else if ( is_special_form(form, "or") )
    compile_or(rls, form, code, flags, line);

  else if ( is_special_form(form, "do") )
    compile_do(rls, form, code, flags, line);

  else if ( is_special_form(form, "control") )
    compile_control(rls, form, code, flags, line);

  else if ( is_special_form(form, "raise") )
    compile_raise(rls, form, code, flags, line);

  else if ( is_special_form(form, "fn") )
    compile_fn(rls, form, code, flags, line);

  else {
    int argc = form->count-1;

    while ( form->count > 0 ) {
      Expr arg = form->head;
      compile_expr(rls, arg, code, flags, line);
      form = form->tail;
    }

    emit_instr(rls, code, OP_CALL, argc);
  }
}

void compile_expr(RlState* rls, Expr x, Chunk* code, int* flags, int* line) {
  if ( is_sym(x) ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) ) // keywords (symbols whose names begin with ':') are treated as literals
      compile_literal(rls, x, code, flags, line);

    else
      compile_reference(rls, s, code, flags, line);

  } else if ( is_list(x) ) {
    List* l = as_list(x);

    if ( l->count == 0 ) // empty list is treated as a literal
      compile_literal(rls, x, code, flags, line);

    else
      compile_funcall(rls, l, code, flags, line);

  } else
    compile_literal(rls, x, code, flags, line);
}

// External -------------------------------------------------------------------
Fun* toplevel_compile(RlState* rls, Expr x) {
  // invoked from the repl and by the compile builtin
  Sym* n; Fun* f; Chunk* c; Method* m; Str* s;
  int fl = CF_ALLOW_DEF;

  save_error_state(rls, 0);

  if ( rl_setjmp(rls) ) {
    restore_error_state(rls);
    f = NULL; // signal failure
  } else {
    StackRef top = rls->s_top;
    stack_push(rls, x);
    n = mk_sym_s(rls, "<toplevel>");
    s = mk_str_s(rls, "<toplevel>");
    f = mk_fun_s(rls, n, false, false);
    c = mk_chunk_s(rls, NULL, n, s);
    compile_expr(rls, x, c, &fl, NULL);
    finalize_chunk(rls, c, 1);
    m = mk_user_method_s(rls, f, 0, false, c);
    fun_add_method(rls, f, m);
    rls->s_top = top;
  }

  discard_error_state(rls);
  return f;
}

Fun* compile_file(RlState* rls, char* fname) {
  Str* file; Sym* name; Fun* fun; Chunk* code; Method* method;
  int flags = CF_ALLOW_DEF, line = 1;
  Port* stream = NULL;
  Expr x;

  save_error_state(rls, 0);

  if ( rl_setjmp(rls) ) {
    restore_error_state(rls);

    if ( stream != NULL )
      close_port(stream);

    fun = NULL; // signal failure
  } else {
    StackRef top = rls->s_top;
    stream = open_port_s(rls, fname, "r");
    file = mk_str_s(rls, strrchr(fname, '/')+1);
    name = mk_sym_s(rls, "<module>");
    code = mk_chunk_s(rls, NULL, name, file);
    fun = mk_fun_s(rls, name, false, false);

    while ( (x=read_exp(rls, stream, &line)) != EOS ) {
      // emit the pop instruction at the top of the loop since we can't tell
      // when the file has one more expression left
      if ( chunk_codec(code) > 0 ) {
        if ( flags & CF_NO_POP )
          flags &= ~CF_NO_POP;

        else
          emit_instr(rls, code, OP_POP);
      }

      // compile the expression
      compile_expr(rls, x, code, &flags, &line);
    }

    // finalize code
    finalize_chunk(rls, code, line);
    method = mk_user_method_s(rls, fun, 0, false, code);
    fun_add_method(rls, fun, method);

    // cleanup
    close_port(stream);

    // reset stack
    rls->s_top = top;

#ifdef RASCAL_DEBUG
    // disassemble_method(method);
#endif
  }

  discard_error_state(rls);
  return fun;
}
