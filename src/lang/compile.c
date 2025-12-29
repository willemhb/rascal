#include <string.h>

#include "util/util.h"
#include "lang/compile.h"
#include "lang/exec.h"
#include "lang/read.h"
#include "lang/print.h"
#include "vm.h"
#include "val.h"

// Globals --------------------------------------------------------------------
// compiler flags
enum {
  CF_NO_POP  = 0x01,
  CF_ALLOW_DEF = 0x02,
  CF_TOPLEVEL_FORM = 0x04,
  CF_TAIL_POSITION = 0x80,
};

// Prototypes -----------------------------------------------------------------
// Helpers --------------------------------------------------------------------
void* syntax_as_obj_s(RlState* rls, List* form, Type* e, Expr x);
Sym* syntax_as_sym_s(RlState* rls, List* form, Expr x);
List* syntax_as_list_s(RlState* rls, List* form, Expr x);
Fun* syntax_as_fun_s(RlState* rls, List* form, Expr x);

void emit_instr(RlState* rls, Chunk* chunk, OpCode op, ...);
void get_ref_instr(Ref* r, OpCode* op, int* arg);
void get_set_instr(Ref* r, OpCode* op, int* arg);
void fill_instr(RlState* rls, Chunk* chunk, int offset, int val);
int add_chunk_constant(RlState* rls, Chunk* chunk, Expr c);
bool is_special_form(List* form, char* form_name);
bool is_syntax(Expr x, char* stx_kw);
void add_name_to_env(RlState* rls, List* form, Env* vars, bool tl, bool m, bool f);
void prepare_local_env(RlState* rls, List* form, Chunk* chunk, int* argc, bool* va);
void process_inner_defs(RlState* rls, List* form, Chunk* chunk, int* flags, int* line, Expr* buf);
Expr get_macro_expansion(RlState* rls, List* form, Env* vars, Ref* macro_ref);
Sym* get_assignment(RlState* rls, List* form);

Ref* list_is_macro_call(RlState* rls, List* form, Env* vars);
Ref* expr_is_macro_call(RlState* rls, Expr expr, Env* vars);

#define is_macro_call(rls, x, v)                \
  c_generic((x),                                \
          List*:list_is_macro_call,             \
          Expr:expr_is_macro_call)(rls, x, v)

//// Dispatch -------------------------------------------------------------------
void compile_quote(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_def(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_def_stx(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_def_multi(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_def_method(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_put(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_if(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_and(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_or(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_do(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_control(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_raise(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_fn(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_module(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);

void compile_closure(RlState* rls, List* form, Chunk* pchunk, Fun* fun, int* flags, int* line);
void compile_sequence(RlState* rls, List* exprs, Chunk* chunk, int* flags, int* line);
void compile_literal(RlState* rls, Expr x, Chunk* chunk);
void compile_reference(RlState* rls, Sym* s, Chunk* chunk);
void compile_global_assignment(RlState* rls, void* ob, Chunk* chunk);
void compile_global_reference(RlState* rls, void* o, Chunk* chunk);
void compile_funcall(RlState* rls, List* form, Chunk* chunk, int* flags, int* line);
void compile_expr(RlState* rls, Expr x, Chunk* chunk, int* flags, int* line);

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
  int b = 1, n = op_arity(op);
  va_list va;

  va_start(va, op);

  for ( int i=0; i < n; i++ ) {
    buffer[i+1]  = va_arg(va, int);
    b++;
  }

  va_end(va);
  code_buf_write(rls, &chunk->code, buffer, b);
}

void get_ref_instr(Ref* ref, OpCode* op, int* arg) {
  *arg = ref->offset;

  if ( ref->ref_type == REF_LOCAL )
    *op = OP_GET_LOCAL;

  else if ( ref->ref_type == REF_LOCAL_UPVAL )
    *op = OP_GET_UPVAL;

  else if ( ref->ref_type == REF_CAPTURED_UPVAL )
    *op = OP_GET_UPVAL;

  else if ( ref->ref_type == REF_GLOBAL )
    *op = OP_GET_GLOBAL;

  else {
    unreachable();
  }
}

void get_set_instr(Ref* ref, OpCode* op, int* arg) {
  *arg = ref->offset;

  if ( ref->ref_type == REF_LOCAL )
    *op = OP_SET_LOCAL;

  else if ( ref->ref_type == REF_LOCAL_UPVAL )
    *op = OP_SET_UPVAL;

  else if ( ref->ref_type == REF_CAPTURED_UPVAL )
    *op = OP_SET_UPVAL;

  else if ( ref->ref_type == REF_GLOBAL )
    *op = OP_SET_GLOBAL;

  else {
    unreachable();
  }
}

void fill_instr(RlState* rls, Chunk* chunk, int offset, int val) {
  (void)rls;

  chunk_code(chunk)[offset] = val;
}

int add_chunk_constant(RlState* rls, Chunk* chunk, Expr c) {
  exprs_push(rls, &chunk->vals, c);

  return chunk_valsc(chunk) - 1;
}

bool is_special_form(List* form, char* f_name) {
  Expr hd = form->head;

  return is_sym(hd) && sym_val_eql(as_sym(hd), f_name);
}

bool is_syntax(Expr x, char* stx_kw) {
  return is_sym(x) && sym_val_eql(as_sym(x), stx_kw);
}

void add_name_to_env(RlState* rls, List* form, Env* vars, bool tl, bool m, bool f) {
  bool a; Sym* n;
  syntax_require_argco(rls, form, 2);
  syntax_require(rls, !(tl && is_local_env(vars)), form, "local definitions not supported.");
  n = get_assignment(rls, form);
  env_define(rls, vars, n, m, f, &a);
  syntax_require(rls, a, form, "redefinition of %s", sym_val(n));
}

void prepare_local_env(RlState* rls, List* form, Chunk* chunk, int* argc, bool* va) {
  bool a; Env* vars; List* argl; Expr x; Sym* n;
 
  *va = false;
  *argc = 0;
  vars = chunk->vars;
  argl = syntax_as_list_s(rls, form, list_snd(form));

  while ( argl->count > 0 ) {
    x = argl->head;
    n = syntax_as_sym_s(rls, form, x);

    // check for & rest parameter marker
    if ( sym_val_eql(n, "&") ) {
      argl = argl->tail;
      syntax_require(rls, argl->count == 1, form,
                     "& not followed by exactly one parameter");
      x = argl->head;
      n = syntax_as_sym_s(rls, form, x);
      *va = true;
    }

    env_define(rls, vars, n, false, false, &a);

    // ensure uniqueness
    syntax_require(rls, a, form, "duplicate parameter name %s", sym_val(n));

    if ( !(*va) )
      (*argc)++;

    argl = argl->tail;
  }
}

void process_inner_defs(RlState* rls, List* form, Chunk* chunk, int* flags, int* line, Expr* buf) {
  // process definitions at the head of the function body
  // all bindings need to be visible to the other internal definitions,
  // so these need to be added to the environment before the definitions
  // are compiled
  List* fbody, *fform; StackRef base; Env* vars; Ref* r; Expr x;

  fbody = form->tail->tail;
  *buf = tag_obj(fbody);
  base = rls->s_top;
  vars = chunk->vars;

  while ( fbody->count > 0 ) {
    x = fbody->head;

    if ( !is_list(x) )
      break;

    fform = as_list(x);

    if ( (r=is_macro_call(rls, fform, vars)) ) {
      x = get_macro_expansion(rls, fform, vars, r);
      stack_push(rls, x);
      fbody = cons(rls, x, fbody->tail);
      *buf = tag_obj(fbody);
      stack_pop(rls);
      continue;
    } else if ( is_special_form(fform, "def") ) {
      add_name_to_env(rls, fform, chunk->vars, false, false, false);
      stack_push(rls, x);
      fbody = fbody->tail;
      *buf = tag_obj(fbody);
    } else {
      break;
    }
  }

  // compile the def forms in the order in which they appeared
  for ( StackRef fb=base; fb<rls->s_top; fb++ ) {
    fform = as_list(*fb);
    // handles updating line numbers
    // easy target for optimization when it comes to that
    compile_funcall(rls, fform, chunk, flags, line);
  }

  // restore stack state
  rls->s_top = base;
}

// Check if a form is a macro call and return the macro Ref if so
Ref* list_is_macro_call(RlState* rls, List* form, Env* vars) {
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

Ref* expr_is_macro_call(RlState* rls, Expr x, Env* vars) {
  if ( !is_list(x) )
    return NULL;

  List* form = as_list(x);

  if ( !is_sym(form->head) )
    return NULL;

  Sym* s = as_sym(form->head);
  Ref* r = env_resolve(rls, vars, s, false);

  if (r != NULL && r->macro)
    return r;

  return NULL;
}

// Expand a macro call at compile time
Expr get_macro_expansion(RlState* rls, List* form, Env* vars, Ref* macro_ref) {
  List* args; int argc; Expr result; StackRef top;

  top = rls->s_top; // save call stack
  stack_check_limit(rls, 3 + form->tail->count);
  // push function and implicit arguments (expanding form and environment)
  stack_preserve(rls, 3, macro_ref->val, tag_obj(form), tag_obj(vars));
  /* The macro receives: whole form, environment,
     then individual arguments (all unevaluated) */
  args = form->tail;
  argc = 2 + push_list(rls, args);
  result = exec_code(rls, argc, 0); // call macro
  rls->s_top = top; // restore call stack

  if ( is_list(result) ) // try to propagate more reliable line information
    as_list(result)->line = form->line;

  return result;
}

Sym* get_assignment(RlState* rls, List* form) {
  Sym* n = syntax_as_sym_s(rls, form, list_snd(form));
  syntax_require(rls, !is_keyword(n), form,
                 "can't assign to keyword %s", sym_val(n));

  return n;
}

// Dispatch -------------------------------------------------------------------
void compile_quote(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  (void)flags;
  (void)line;

  syntax_require_argco(rls, form, 1);
  Expr x = form->tail->head;
  compile_literal(rls, x, chunk);
}

void compile_def(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  int candef, arg; bool a; OpCode op; Sym* n; Ref* r; Expr x;

  candef = *flags & CF_ALLOW_DEF;
  syntax_require(rls, candef, form, "`def` not allowed in current context.");

  if ( is_global_env(chunk->vars) ) {
    // validation and definition not done elsewhere
      syntax_require_argco(rls, form, 2);
      n = get_assignment(rls, form);
      r = env_define(rls, chunk->vars, n, false, false, &a);
      syntax_require(rls, a, form,
                     "redefining symbol %s.", sym_val(n));
  } else {
    n = as_sym(list_snd(form));
    r = env_resolve(rls, chunk->vars, n, false);
    syntax_require(rls, r != NULL, form, "unbound symbol %s.", sym_val(n));
  }

  *flags &= ~CF_ALLOW_DEF; // temporarily disable to avoid goofy shit
  x = list_thd(form);      // binding

  if ( is_list(x) && is_special_form(as_list(x), "fn") ) {
    List* fn_form; Fun* def_fun;
    r->final = true; // function bindings always immutable
    fn_form = as_list(x);
    def_fun = mk_fun_s(rls, n, false, false);
    compile_closure(rls, fn_form, chunk, def_fun, flags, line);
    stack_pop(rls);
  } else {
    compile_expr(rls, x, chunk, flags, line);
  }

  if ( r->ref_type == REF_GLOBAL ) {
    compile_global_assignment(rls, r, chunk);
  } else {
  get_set_instr(r, &op, &arg);
  emit_instr(rls, chunk, op, arg); 
  }

  *flags |= CF_ALLOW_DEF;
}

void compile_def_stx(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  Env* vars; Sym* n; List* fn_form; Ref* r; Fun* macro_fun; bool a;

  vars = chunk->vars;
  syntax_require(rls, is_global_env(vars), form, "local macros not supported.");
  syntax_require_argco(rls, form, 2);
  n = get_assignment(rls, form);
  fn_form = syntax_as_list_s(rls, form, list_thd(form));
  syntax_require(rls, is_special_form(fn_form, "fn"),
                 form, "syntax transformer is not a `fn` form.");
  // Define the macro in the environment and mark it as a macro (ensure the binding doesn't already exist)
  r = env_define(rls, vars, n, true, true, &a);
  syntax_require(rls, a, form, "%s already bound.", sym_val(n));
  // create function to hold method and assign to ref immediately (not at run time)
  macro_fun = mk_fun(rls, n, true, true);
  r->val = tag_obj(macro_fun);
  // compile_closure handles adding the method based on what kind of fun it's passed
  compile_closure(rls, fn_form, chunk, macro_fun, flags, line);
  *flags |= CF_NO_POP; // signal don't emit a pop instruction
}

void compile_def_multi(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  Env* vars; Sym* n; List* fn_form; Ref* r; Fun* g_fun; bool a;

  vars = chunk->vars;
  syntax_require_argco(rls, form, 2);
  syntax_require(rls, is_global_env(vars), form, "local generics not supported." );
  n = get_assignment(rls, form);
  fn_form = syntax_as_list_s(rls, form, list_thd(form));
  syntax_require(rls, is_special_form(fn_form, "fn"), form, "method is not a `fn` form.");
  // Define the function in the environment and mark it as a generic (ensure the binding doesn't already exist)
  r = env_define(rls, vars, n, false, true, &a);
  syntax_require(rls, a, form, "%s already bound.", sym_val(n));
  // create function to hold method and assign to ref immediately (not at run time)
  g_fun = mk_fun(rls, n, false, true);
  r->val = tag_obj(g_fun);

  // compile_closure handles adding the method based on what kind of fun it's passed
  compile_closure(rls, fn_form, chunk, g_fun, flags, line);
}

void compile_def_method(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  Env* vars; Sym* n; List* fn_form; Ref* r; Fun* g_fun;

  vars = chunk->vars;
  syntax_require_argco(rls, form, 2);
  syntax_require(rls, is_global_env(vars), form, "local generics not supported.");
  n = get_assignment(rls, form);
  fn_form = syntax_as_list_s(rls, form, list_thd(form));
  syntax_require(rls, is_special_form(fn_form, "fn"),
                 form, "method is not a `fn` form.");
  // Resolve the function binding (ensure it exists)
  r = env_resolve(rls, vars, n, false);
  syntax_require(rls, r != NULL, form, "%s not bound to generic.", sym_val(n));
  g_fun = syntax_as_fun_s(rls, form, r->val);
  syntax_require(rls, g_fun->generic, form,
                 "%s does not support overloads.", sym_val(n));
  // compile_closure handles adding the method based on what kind of fun it's passed
  compile_closure(rls, fn_form, chunk, g_fun, flags, line);
}

void compile_put(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  Sym* n; Ref* r; OpCode op; int arg; Env* vars;

  syntax_require_argco(rls, form, 2);
  vars = chunk->vars;
  n = get_assignment(rls, form);
  r = env_resolve(rls, vars, n, false);

  // compile the expression
  compile_expr(rls, list_thd(form), chunk, flags, line);

  if ( r == NULL ) { // lookup once at runtime and cache reference
    compile_global_assignment(rls, n, chunk);
  } else if ( r->ref_type == REF_GLOBAL ) { // validate and cache reference now
    syntax_require(rls, !r->final, form,
                   "illegal assignmen to final reference %s", sym_val(n));
    compile_global_assignment(rls, r, chunk);
  } else {
    get_set_instr(r, &op, &arg);
    emit_instr(rls, chunk, op, arg);
  }
}

void compile_if(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  syntax_require_vargco(rls, form, 2);
  List* exprs = form->tail;
  int exprc = exprs->count;
  int testc = exprc / 2;
  // keep track of offsets that need to be filled in once the total Chunk size is known
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
      compile_expr(rls, test, chunk, flags, line);
      emit_instr(rls, chunk, OP_JUMP_F, 0);
      off1 = chunk_codec(chunk);
      compile_expr(rls, then, chunk, flags, line);
      emit_instr(rls, chunk, OP_JUMP, 0);
      off2 = chunk_codec(chunk);
      fill_instr(rls, chunk, off1-1, off2-off1);
      then_offsets[i] = off2; // save to fill in once full Chunk size is known
    }
  }
 
  compile_expr(rls, last_expr, chunk, flags, line);
  end = chunk_codec(chunk);

  // fill in missing jump offsets
  for ( int i=0; i<testc; i++ ) {
    int off = then_offsets[i];
    fill_instr(rls, chunk, off-1, end-off);
  }
}

void compile_and(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  List* exprs = form->tail;
  int exprc = exprs->count;

  if ( exprc == 0 )
    emit_instr(rls, chunk, OP_TRUE);

  else {
    // keep track of offsets that need to be filled in once the total Chunk size is known
    int offsets[exprc-1];

    for ( int i=0; i<exprc-1; i++ ) {
      compile_expr(rls, exprs->head, chunk, flags, line);
      emit_instr(rls, chunk, OP_PJUMP_F, 0);
      offsets[i] = chunk_codec(chunk);
      exprs = exprs->tail;
    }

    compile_expr(rls, list_fst(exprs), chunk, flags, line);

    int end = chunk_codec(chunk);

    for ( int i=0; i<exprc-1; i++ ) {
      int offset = offsets[i];

      fill_instr(rls, chunk, offset-1, end-offset);
    }
  }
}

void compile_or(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  List* exprs = form->tail;
  int exprc = exprs->count;

  if ( exprc == 0 )
    emit_instr(rls, chunk, OP_FALSE);

  else {
    // keep track of offsets that need to be filled in once the total Chunk size is known
    int offsets[exprc-1];

    for ( int i=0; i<exprc-1; i++ ) {
      compile_expr(rls, exprs->head, chunk, flags, line);
      emit_instr(rls, chunk, OP_PJUMP_T, 0);
      offsets[i] = chunk_codec(chunk);
      exprs = exprs->tail;
    }

    compile_expr(rls, list_fst(exprs), chunk, flags, line);

    int end = chunk_codec(chunk);

    for ( int i=0; i<exprc-1; i++ ) {
      int offset = offsets[i];

      fill_instr(rls, chunk, offset-1, end-offset);
    }
  }
}

void compile_do(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  syntax_require_vargco(rls, form, 1);
  compile_sequence(rls, form->tail, chunk, flags, line);
}

void compile_control(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  Sym* catch_name, *body_name; Fun* catch_fun, *body_fun;
  List* body_form, *catch_form; StackRef top;

  syntax_require_argco(rls, form, 2);
  top = rls->s_top; // save stack
  body_form = syntax_as_list_s(rls, form, list_snd(form));
  catch_form = syntax_as_list_s(rls, form, list_thd(form));
  syntax_require(rls, is_special_form(body_form, "fn"), body_form,
                 "control body must be a `fn` form.");
  syntax_require(rls, is_special_form(catch_form, "fn"), catch_form,
                 "control handler must be a `fn` form.");
  catch_name = mk_sym_s(rls, "<control-handler>");
  catch_fun = mk_fun_s(rls, catch_name, false, false);
  body_name = mk_sym_s(rls, "<control-body>");
  body_fun = mk_fun_s(rls, body_name, false, false);

  // compile both closures so that the final result is catch and body on stack
  // in that order
  compile_closure(rls, catch_form, chunk, catch_fun, flags, line);
  compile_closure(rls, body_form, chunk, body_fun, flags, line);

  // emit CATCH/ECATCH instructions
  emit_instr(rls, chunk, OP_CATCH);
  emit_instr(rls, chunk, OP_ECATCH);
  rls->s_top = top; // reset stack
}

void compile_raise(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  syntax_require_argco(rls, form, 1);
  compile_expr(rls, list_snd(form), chunk, flags, line);
  emit_instr(rls, chunk, OP_RAISE);
}

void compile_fn(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  Sym* n; Fun* cl_fun; StackRef top;
 
  syntax_require_vargco(rls, form, 2);
  top = rls->s_top;
  n = mk_sym_s(rls, "fn");
  cl_fun = mk_fun_s(rls, n, false, false);
  compile_closure(rls, form, chunk, cl_fun, flags, line);
  rls->s_top = top;
}


void compile_closure(RlState* rls, List* form, Chunk* chunk, Fun* fun, int* flags, int* line) {
  (void)flags;
  syntax_require_vargco(rls, form, 2);

  bool va; int argc, upvc, lflags, linec, *linebuf;
  StackRef top, buf; Chunk* lchunk; Method* m;

  lflags = CF_ALLOW_DEF;
  linec  = line ? 1 : form->line;
  linebuf = line ? &linec : NULL;
  top = rls->s_top;
  lchunk = mk_chunk_s(rls, chunk->vars, fun->name, chunk->file);

  // validate arguments and prepare environment
  prepare_local_env(rls, form, lchunk, &argc, &va);
  buf = stack_push(rls, NUL);
  // handle inner definitions at top of function body
  process_inner_defs(rls, form, lchunk, &lflags, linebuf, buf);

  // definitions only allowed at the top of a function body
  lflags &= ~CF_ALLOW_DEF;

  // compile the body and remove it from the stack
  compile_sequence(rls, as_list(*buf), lchunk, &lflags, linebuf);
  stack_pop(rls);
  finalize_chunk(rls, lchunk, linec);
  m = mk_user_method_s(rls, fun, argc, va, lchunk);

  // what to do with the compiled closure depends on the flags of fun
  if ( fun->macro ) { // macros should be evaluated immediately
    fun_add_method(rls, fun, m);
  }

  else { // closure must be handled at runtime, store method in constants
    compile_literal(rls, tag_obj(m), chunk);
    upvc = method_upvc(m);

    if ( upvc > 0 ) {
      emit_instr(rls, chunk, OP_CLOSURE, upvc);

      instr_t buffer[upvc*2];

      for ( int i=0, j=0; i < env_upval_maxc(lchunk->vars) && j < upvc; i++ ) {
        EMapKV* kv = &env_upval_refs(lchunk->vars)[i];
       
        if ( kv->key == NULL )
          continue;
       
        Ref* r = kv->val;
        buffer[j*2] = r->ref_type == REF_LOCAL_UPVAL;
        buffer[j*2+1] = r->captures->offset;
        j++;
      }

      // write arguments to closure at once
      code_buf_write(rls, &lchunk->code, buffer, upvc*2);
    }

    // this handles binding the method to its corresponding function object
    emit_instr(rls, chunk, OP_ADD_METHOD);
  }

  // reset stack
  rls->s_top = top;
}

void compile_sequence(RlState* rls, List* xprs, Chunk* chunk, int* flags, int* line) {
  Expr x;
 
  while ( xprs->count > 0 ) {
    *flags &= ~CF_NO_POP; // reset NO_POP flag if necessary
    x = xprs->head;
    compile_expr(rls, x, chunk, flags, line);

    if ( xprs->count > 1 && (*flags & CF_NO_POP) != CF_NO_POP )
      emit_instr(rls, chunk, OP_POP);

    xprs = xprs->tail;
  }
}

void compile_literal(RlState* rls, Expr x, Chunk* chunk) {
  if ( is_num(x) ) {
    if ( x == RL_ZERO )
      emit_instr(rls, chunk, OP_ZERO);

    else if ( x == RL_ONE )
      emit_instr(rls, chunk, OP_ONE);

    else {
      Num n = as_num(x);

      if ( is_int(n) && n <= INT16_MAX && n >= INT16_MIN )
        emit_instr(rls, chunk, OP_SMALL, (short)n);

      else
        goto fallback;
    }
  } else if ( is_glyph(x) ) {
    if ( x == EOS )
      emit_instr(rls, chunk, OP_EOS);

    else
      emit_instr(rls, chunk, OP_GLYPH, as_glyph(x));
  } else if ( x == TRUE ) {
    emit_instr(rls, chunk, OP_TRUE);
  } else if ( x == FALSE ) {
    emit_instr(rls, chunk, OP_FALSE);
  } else if ( x == NUL ) {
    emit_instr(rls, chunk, OP_NUL);
  } else {
  fallback:
    int n = add_chunk_constant(rls, chunk, x);

    emit_instr(rls, chunk, OP_GET_VALUE, n);
  }
}

void compile_global_assignment(RlState* rls, void* ob, Chunk* chunk) {
  int n = add_chunk_constant(rls, chunk, tag_obj(ob));
  emit_instr(rls, chunk, OP_SET_GLOBAL, n);
}

void compile_global_reference(RlState* rls, void* ob, Chunk* chunk) {
  int n = add_chunk_constant(rls, chunk, tag_obj(ob));
  emit_instr(rls, chunk, OP_GET_GLOBAL, n);
}

void compile_reference(RlState* rls, Sym* s, Chunk* chunk) {
  Ref* r = env_resolve(rls, chunk->vars, s, false);

  if ( r == NULL ) {
    compile_global_reference(rls, s, chunk);
  } else if ( r->ref_type == REF_GLOBAL ) {
    compile_global_reference(rls, r, chunk);
  } else {
    OpCode op; int arg;
    get_ref_instr(r, &op, &arg);
    emit_instr(rls, chunk, op, arg);
  }
}

void compile_funcall(RlState* rls, List* form, Chunk* chunk, int* flags, int* line) {
  assert(form->count > 0);

  if ( line != NULL && form->line > *line ) {
    add_line_number(rls, chunk, *line);
    *line = form->line;
  }

  Env* vars = chunk->vars;
  // Check for macro calls first - macros are expanded before other processing
  Ref* macro_ref = is_macro_call(rls, form, vars);

  if (macro_ref != NULL) {
    Expr expanded = get_macro_expansion(rls, form, vars, macro_ref);
    stack_push(rls, expanded); // save expanded expression
    compile_expr(rls, expanded, chunk, flags, line);
    stack_pop(rls); // unsave expanded expression
  }

  else if ( is_special_form(form, "quote") )
    compile_quote(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "def") )
    compile_def(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "def-stx") )
    compile_def_stx(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "def-multi") )
    compile_def_multi(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "def-method") )
    compile_def_method(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "put") )
    compile_put(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "if") )
    compile_if(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "and") )
    compile_and(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "or") )
    compile_or(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "do") )
    compile_do(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "control") )
    compile_control(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "raise") )
    compile_raise(rls, form, chunk, flags, line);

  else if ( is_special_form(form, "fn") )
    compile_fn(rls, form, chunk, flags, line);

  else {
    int argc = form->count-1;

    while ( form->count > 0 ) {
      Expr arg = form->head;
      compile_expr(rls, arg, chunk, flags, line);
      form = form->tail;
    }

    emit_instr(rls, chunk, OP_CALL, argc);
  }
}

void compile_expr(RlState* rls, Expr x, Chunk* chunk, int* flags, int* line) {
  if ( is_sym(x) ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) ) // keywords (symbols whose names begin with ':') are treated as literals
      compile_literal(rls, x, chunk);

    else
      compile_reference(rls, s, chunk);

  } else if ( is_list(x) ) {
    List* l = as_list(x);

    if ( l->count == 0 ) // empty list is treated as a literal
      compile_literal(rls, x, chunk);

    else
      compile_funcall(rls, l, chunk, flags, line);

  } else
    compile_literal(rls, x, chunk);
}

// External -------------------------------------------------------------------
Fun* toplevel_compile(RlState* rls, Expr x) {
  // invoked from the repl and by the compile builtin
  Sym* n; Fun* f; Chunk* c; Method* m; Str* s;
  int fl = CF_ALLOW_DEF | CF_TOPLEVEL_FORM;

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
#ifdef RASCAL_DEBUG
  // disassemble_method(f->method);
#endif
  return f;
}

Fun* compile_file(RlState* rls, char* fname) {
  Str* file; Sym* name; Fun* fun; Chunk* chunk; Method* method;
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
    chunk = mk_chunk_s(rls, NULL, name, file);
    fun = mk_fun_s(rls, name, false, false);

    while ( (x=read_exp(rls, stream, &line)) != EOS ) {
      // emit the pop instruction at the top of the loop since we can't tell
      // when the file has one more expression left
      if ( chunk_codec(chunk) > 0 ) {
        if ( flags & CF_NO_POP )
          flags &= ~CF_NO_POP;

        else
          emit_instr(rls, chunk, OP_POP);
      }

      // compile the expression
      compile_expr(rls, x, chunk, &flags, &line);
    }

    // finalize Chunk
    finalize_chunk(rls, chunk, line);
    method = mk_user_method_s(rls, fun, 0, false, chunk);
    fun_add_method(rls, fun, method);

    // cleanup
    close_port(stream);

    // reset stack
    rls->s_top = top;
  }

#ifdef RASCAL_DEBUG
  //  if ( fun )
  //  disassemble_method(fun->method);
#endif

  discard_error_state(rls);
  return fun;
}
