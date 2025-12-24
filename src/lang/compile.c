#include <string.h>

#include "util/util.h"
#include "lang/compile.h"
#include "lang/exec.h"
#include "lang/read.h"
#include "vm.h"
#include "val.h"

/*
  general compile function signature:

  RlState* rls - state object
  List* form   - the form being compiled
  Env* vars    - the environment in which the expression is compiling
  Alist* vals  - constant store
  Buf16* code  - bytecode
  Fun* fun     - the function to which the compiled code will belong
 */

// Globals --------------------------------------------------------------------
// compiler flags
enum {
  CF_NO_EXPR  = 0x01,
  CF_TAIL_POS = 0x02,
};

// Prototypes -----------------------------------------------------------------
// Helpers --------------------------------------------------------------------
Sym* mk_module_name(RlState* rls, char* fname);
Sym* mk_module_name_s(RlState* rls, char* fname);
void* syntax_as_obj_s(RlState* rls, char* sf, char* fn, Type* e, Expr x);
Sym* syntax_as_sym_s(RlState* rls, char* sf, char* fn, Expr x);
List* syntax_as_list_s(RlState* rls, char* sf, char* fn, Expr x);
Fun* syntax_as_fun_s(RlState* rls, char* sf, char* fn, Expr x);
void emit_instr(RlState* rls, Buf16* code, OpCode op, ...);
void fill_instr(RlState* rls, Buf16* code, int offset, int val);
bool is_special_form(List* form, char* form_name);
void prepare_env(RlState* rls, char* fname, List* argl, Env* vars, int* argc, bool* va);
Ref* is_macro_call(RlState* rls, List* form, Env* vars);
Expr get_macro_expansion(RlState* rls, List* form, Env* vars, Ref* macro_ref);



// Dispatch -------------------------------------------------------------------
void compile_quote(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_def(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_def_stx(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_def_multi(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_def_method(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_put(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_if(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_and(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_or(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_do(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_fn(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);

void compile_closure(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_defs(RlState* rls, List** xbuf, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_sequence(RlState* rls, List* exprs, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_literal(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_reference(RlState* rls, Sym* s, Env* ref, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_funcall(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);
void compile_expr(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags);

// Implementations ------------------------------------------------------------
// Helpers --------------------------------------------------------------------
Sym* mk_module_name(RlState* rls, char* fname) {
  char* fname_start = strrchr(fname, '/')+1;
  char* prefix = "file:";
  int fname_length = strlen(fname_start) - 3; // remove .rl
  int prefix_length = strlen(prefix);
  int buffer_size = fname_length+prefix_length+1;
  char buffer[buffer_size] = {};

  snprintf(buffer, buffer_size, "file:%s", fname_start);

  return mk_sym(rls, buffer);
}

Sym* mk_module_name_s(RlState* rls, char* fname) {
  Sym* out = mk_module_name(rls, fname); push(rls, tag_obj(out));

  return out;
}

void* syntax_as_obj_s(RlState* rls, char* sf, char* fn, Type* e, Expr x) {
  Type* xt = type_of(x);

  syntax_require(rls, sf, fn, xt->tag == e->tag,
                 "wanted a %s, got a %s", type_name(e), type_name(xt));

  return as_obj(x);
}

Sym* syntax_as_sym_s(RlState* rls, char* sf, char* fn, Expr x) {
  return syntax_as_obj_s(rls, sf, fn, &SymType, x);
}

List* syntax_as_list_s(RlState* rls, char* sf, char* fn, Expr x) {
  return syntax_as_obj_s(rls, sf, fn, &ListType, x);
}

Fun* syntax_as_fun_s(RlState* rls, char* sf, char* fn, Expr x) {
  return syntax_as_obj_s(rls, sf, fn, &FunType, x);
}

void emit_instr(RlState* rls, Buf16* code, OpCode op, ...) {
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

  buf16_write(rls, code, buffer, b);
}

void fill_instr(RlState* rls, Buf16* code, int offset, int val) {
  (void)rls;

  ((instr_t*)code->binary.vals)[offset] = val;
}

bool is_special_form(List* form, char* f_name) {
  Expr hd = form->head;

  return is_sym(hd) && sym_val_eql(as_sym(hd), f_name);
}


void prepare_env(RlState* rls, char* fname, List* argl, Env* vars, int* argc, bool* va) {
  *va = false;
  *argc = 0;

  while ( argl->count > 0 ) {
    Expr x = argl->head;
    Sym* n = syntax_as_sym_s(rls, "fn", fname, x);

    // check for & rest parameter marker
    if ( sym_val_eql(n, "&") ) {
      argl = argl->tail;
      require(rls, argl->count == 1, "& must be followed by exactly one parameter");
      n = as_sym_s(rls, "fun", argl->head);
      *va = true;
    }

    bool a;
    env_define(rls, vars, n, false, false, &a);

    // ensure uniqueness
    syntax_require(rls, "fn", fname, a, "duplicate parameter name %s", sym_val(n));

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
  int sp = save_sp(rls);

  // prepare call state on stack
  push(rls, macro_ref->val); // this is the macro function
  push(rls, tag_obj(form));  // form being expanded
  push(rls, tag_obj(vars));  // expansion environment

  // The macro receives: whole form, environment, then individual arguments (all unevaluated)
  List* args = form->tail;
  int argc = 2 + push_list(rls, args);

#ifdef RASCAL_DEBUG
  // stack_report_slice(rls, argc+1, "DEBUG - prepared macro call for %s", sym_val(macro_ref->name));
#endif

  Expr result = exec_code(rls, argc, 0);
  restore_sp(rls, sp);

#ifdef RASCAL_DEBUG
  // printf("\nDEBUG - result of expanding %s: ", sym_val(macro_ref->name));
  // print_exp(&Outs, result);
  // printf("\n");
#endif

  return result;
}

// Dispatch -------------------------------------------------------------------

void compile_quote(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  syntax_require_argco(rls, "quote", fn_name(fun), 1, form);

  Expr x = form->tail->head;

  compile_literal(rls, x, vars, vals, code, fun, flags);
}

void compile_def(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  syntax_require_argco(rls, "def", fn_name(fun), 2, form);

  Sym* n = as_sym_s(rls, "def", form->tail->head);

  syntax_require(rls, "def", fn_name(fun),
                 !is_keyword(n),
                 "can't assign to keyword %s", sym_val(n));

  bool a;

  Ref* r = env_define(rls, vars, n, false, false, &a);

  syntax_require(rls, "def", fn_name(fun), a,
                 "%s already bound in this environment",
                 sym_val(r->name));

  int i  = r->offset;

  OpCode op;

  switch ( r->ref_type ) {
    case REF_GLOBAL:
      op = OP_SET_GLOBAL;
      break;

    case REF_LOCAL:
      op = OP_SET_LOCAL;
      break;

    default:
      unreachable();
  }

  Expr x = list_thd(form);

  if ( is_list(x) && is_special_form(as_list(x), "fn") ) {
    List* fn_form = as_list(x);
    Fun* def_fun = mk_fun_s(rls, n, false, false);
    compile_closure(rls, fn_form, vars, vals, code, def_fun, flags);
    pop(rls);
  } else {
    compile_expr(rls, x, vars, vals, code, fun, flags);
  }
  emit_instr(rls, code, op, i);
}

void compile_def_stx(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  syntax_require(rls, "def-stx", fn_name(fun), is_global_env(vars),
                 "local macros not supported" );
  syntax_require_argco(rls, "def-stx", fn_name(fun), 2, form);

  Sym* n = syntax_as_sym_s(rls, "def-stx", fn_name(fun), form->tail->head);
  List* fn_form = syntax_as_list_s(rls, "def-stx", fn_name(fun), form->tail->tail->head);

  syntax_require(rls, "def-stx", fn_name(fun), !is_keyword(n), "can't assign to keyword %s", sym_val(n));
  syntax_require(rls, "def-stx", fn_name(fun), is_special_form(fn_form, "fn"), "transformer is not a `fn` form");

  // Define the macro in the environment and mark it as a macro (ensure the binding doesn't already exist)
  bool a;
  Ref* r = env_define(rls, vars, n, true, true, &a);

  syntax_require(rls, "def-stx", fn_name(fun), a, "%s already bound", sym_val(n));

  // create function to hold method and assign to ref immediately (not at run time)
  Fun* macro_fun = mk_fun(rls, n, true, true);
  r->val = tag_obj(macro_fun);

  // compile_closure handles adding the method based on what kind of fun it's passed
  compile_closure(rls, fn_form->tail, vars, vals, code, macro_fun, flags);
  *flags |= CF_NO_EXPR; // indicate that no expression was added to the stack (prevents emitting a pop instruction)
}

void compile_def_multi(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  syntax_require_argco(rls, "def-multi", fn_name(fun), 2, form);
  syntax_require(rls, "def-multi", fn_name(fun), is_global_env(vars),
                 "local generics not supported" );

  Sym* n = syntax_as_sym_s(rls, "def-multi", fn_name(fun), form->tail->head);
  List* fn_form = syntax_as_list_s(rls, "def-multi", fn_name(fun), form->tail->tail->head);

  syntax_require(rls, "def-multi", fn_name(fun), !is_keyword(n), "can't assign to keyword %s", sym_val(n));
  syntax_require(rls, "def-multi", fn_name(fun), is_special_form(fn_form, "fn"), "transformer is not a `fn` form");

  // Define the function in the environment and mark it as a generic (ensure the binding doesn't already exist)
  bool a;
  Ref* r = env_define(rls, vars, n, false, true, &a);

  syntax_require(rls, "def-multi", fn_name(fun), a, "%s already bound", sym_val(n));

  // create function to hold method and assign to ref immediately (not at run time)
  Fun* generic_fun = mk_fun(rls, n, false, true);
  r->val = tag_obj(generic_fun);

  // compile_closure handles adding the method based on what kind of fun it's passed
  compile_closure(rls, fn_form->tail, vars, vals, code, generic_fun, flags);
}

void compile_def_method(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  syntax_require_argco(rls, "def-method", fn_name(fun), 2, form);
  syntax_require(rls, "def-method", fn_name(fun), is_global_env(vars),
                 "local generics not supported" );

  Sym* n = syntax_as_sym_s(rls, "def-method", fn_name(fun), form->tail->head);
  List* fn_form = syntax_as_list_s(rls, "def-method", fn_name(fun), form->tail->tail->head);

  syntax_require(rls, "def-method", fn_name(fun), !is_keyword(n), "can't assign to keyword %s", sym_val(n));
  syntax_require(rls, "def-method", fn_name(fun), is_special_form(fn_form, "fn"), "transformer is not a `fn` form");

  // Resolve the function binding (ensure it exists)
  Ref* r = env_resolve(rls, vars, n, false);

  syntax_require(rls, "def-method", fn_name(fun), r != NULL, "%s not bound to generic", sym_val(n));

  Fun* g_fun = syntax_as_fun_s(rls, "def-method", fn_name(fun), r->val);

  syntax_require(rls, "def-method", fn_name(fun), g_fun->generic, "%s does not support overloads");

  // compile_closure handles adding the method based on what kind of fun it's passed
  compile_closure(rls, fn_form->tail, vars, vals, code, g_fun, flags);
}

void compile_put(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  syntax_require_argco(rls, "put", fn_name(fun), 2, form);
  Sym* n = syntax_as_sym_s(rls, "put", fn_name(fun), form->tail->head);
  syntax_require(rls, "put", fn_name(fun), !is_keyword(n), "can't assign to keyword %s", sym_val(n));
  Ref* r = env_resolve(rls, vars, n, false);
  require(rls, r != NULL, "can't assign to %s before it's defined", sym_val(n));

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
      break;

    default:
      unreachable();
  }

  compile_expr(rls, list_thd(form), vars, vals, code, fun, flags);
  emit_instr(rls, code, op, i);
}

void compile_if(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  syntax_require_argco2(rls, "if", fn_name(fun), 2, 3, form);

  Expr test = list_snd(form);
  Expr then = list_thd(form);
  Expr alt  = form->count == 3 ? NUL : list_fth(form);

  // compile different parts of the form, saving offsets to fill in later
  compile_expr(rls, test, vars, vals, code, fun, flags);
  emit_instr(rls, code, OP_JUMP_F, 0);
  int off1 = code->binary.count;
  compile_expr(rls, then, vars, vals, code, fun, flags);
  emit_instr(rls, code, OP_JUMP, 0);
  int off2 = code->binary.count;
  compile_expr(rls, alt, vars, vals, code, fun, flags);
  int off3 = code->binary.count;

  fill_instr(rls, code, off1-1, off2-off1);
  fill_instr(rls, code, off2-1, off3-off2);
}

void compile_and(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  List* exprs = form->tail;
  int exprc = exprs->count;

  if ( exprc == 0 )
    emit_instr(rls, code, OP_TRUE);

  else {
    // keep track of offsets that need to be filled in once the total code size is known
    int offsets[exprc-1];

    for ( int i=0; i<exprc-1; i++ ) {
      compile_expr(rls, exprs->head, vars, vals, code, fun, flags);
      emit_instr(rls, code, OP_PJUMP_F, 0);
      offsets[i] = code->binary.count;
      exprs = exprs->tail;
    }

    compile_expr(rls, list_fst(exprs), vars, vals, code, fun, flags);

    int end = code->binary.count;

    for ( int i=0; i<exprc-1; i++ ) {
      int offset = offsets[i];

      fill_instr(rls, code, offset-1, end-offset);
    }
  }
}

void compile_or(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  List* exprs = form->tail;
  int exprc = exprs->count;

  if ( exprc == 0 )
    emit_instr(rls, code, OP_FALSE);

  else {
    // keep track of offsets that need to be filled in once the total code size is known
    int offsets[exprc-1];

    for ( int i=0; i<exprc-1; i++ ) {
      compile_expr(rls, exprs->head, vars, vals, code, fun, flags);
      emit_instr(rls, code, OP_PJUMP_T, 0);
      offsets[i] = code->binary.count;
      exprs = exprs->tail;
    }

    compile_expr(rls, list_fst(exprs), vars, vals, code, fun, flags);

    int end = code->binary.count;

    for ( int i=0; i<exprc-1; i++ ) {
      int offset = offsets[i];

      fill_instr(rls, code, offset-1, end-offset);
    }
  }
}

void compile_do(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  syntax_require_vargco(rls, "do", fn_name(fun), 1, form);
  compile_sequence(rls, form->tail, vars, vals, code, fun, flags);
}

void compile_fn(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  syntax_require_vargco(rls, "fn", fn_name(fun), 2, form);
  int sp = save_sp(rls);
  Sym* n = mk_sym_s(rls, "fn");
  Fun* cl_fun = mk_fun_s(rls, n, false, false);

  compile_closure(rls, form->tail, vars, vals, code, cl_fun, flags);
  restore_sp(rls, sp);
}

void compile_closure(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  syntax_require_vargco(rls, "fn", fn_name(fun), 1, form); // kludgey, should be 2. fix this

  bool va;
  int sp = save_sp(rls), argc, upvc;
  List* body = form->tail;
  List* argl = syntax_as_list_s(rls, "fn", fn_name(fun), form->head);
  Env* lvars = mk_env_s(rls, vars);
  Alist* lvals = mk_alist_s(rls);
  Buf16* lcode = mk_buf16_s(rls);

  // validate arguments and prepare environment
  prepare_env(rls, fn_name(fun), argl, lvars, &argc, &va);
  compile_defs(rls, &body, lvars, lvals, lcode, fun, flags);
  compile_sequence(rls, body, lvars, lvals, lcode, fun, flags);

  upvc = lvars->upvs.count;

  if ( upvc > 0 )
    emit_instr(rls, lcode, OP_CAPTURE);

  emit_instr(rls, lcode, OP_RETURN);

  Chunk* chunk = mk_chunk_s(rls, lvars, lvals, lcode);
  Method* method = mk_user_method_s(rls, fun, argc, va, chunk);

  // what to do with the compiled closure depends on the flags of fun
  if ( fun->macro ) // macros should be evaluated immediately
    fun_add_method(rls, fun, method);

  else { // closure must be handled at runtime, store method in constants
    compile_literal(rls, tag_obj(method), vars, vals, code, fun, flags);

    if ( upvc > 0 ) {
      emit_instr(rls, code, OP_CLOSURE, upvc);

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
      buf16_write(rls, code, buffer, upvc*2);
    }

    // this handles binding the method to its corresponding function object
    emit_instr(rls, code, OP_ADD_METHOD);
  }

  restore_sp(rls, sp); // restore stack
}

// compile internal definitions at the top of a function body before compiling the rest
void compile_defs(RlState* rls, List** xbuf, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  List* xprs = *xbuf;

  while ( xprs->count > 0 ) {
    Expr head = xprs->head;

    if ( is_list(head) ) {
      List* hx = as_list(head);

      if ( is_special_form(hx, "def") ) {
        compile_def(rls, hx, vars, vals, code, fun, flags);
        xprs = xprs->tail;
        continue;
      }
    }

    break;
  }

  *xbuf = xprs;
}

void compile_sequence(RlState* rls, List* xprs, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  while ( xprs->count > 0 ) {
    *flags &= ~CF_NO_EXPR; // reset NO_EXPR flag if necessary
    Expr x = xprs->head;
    compile_expr(rls, x, vars, vals, code, fun, flags);

    if ( xprs->count > 1 && (*flags & CF_NO_EXPR) != CF_NO_EXPR )
      emit_instr(rls, code, OP_POP);

    xprs = xprs->tail;
  }
}

void compile_literal(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  (void)vars;
  (void)fun;
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
    emit_instr(rls, code, OP_GLYPH, as_glyph(x));
  } else if ( x == TRUE ) {
    emit_instr(rls, code, OP_TRUE);
  } else if ( x == FALSE ) {
    emit_instr(rls, code, OP_FALSE);
  } else if ( x == NUL ) {
    emit_instr(rls, code, OP_NUL);
  } else {
  fallback:
    int n = alist_push(rls, vals, x);

    emit_instr(rls, code, OP_GET_VALUE, n-1);
  }
}

void compile_reference(RlState* rls, Sym* s, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  (void)vals;
  (void)flags;

  Ref* r = env_resolve(rls, vars, s, false);

  require(rls, r != NULL, "undefined variable %s in %s", sym_val(s), fn_name(fun));

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

void compile_funcall(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  assert(form->count > 0);

  // Check for macro calls first - macros are expanded before other processing
  Ref* macro_ref = is_macro_call(rls, form, vars);

  if (macro_ref != NULL) {
    Expr expanded = get_macro_expansion(rls, form, vars, macro_ref);
    push(rls, expanded); // save expanded expression
    compile_expr(rls, expanded, vars, vals, code, fun, flags);
    pop(rls); // unsave expanded expression
  }

  else if ( is_special_form(form, "quote") )
    compile_quote(rls, form, vars, vals, code, fun, flags);

  else if ( is_special_form(form, "def") )
    compile_def(rls, form, vars, vals, code, fun, flags);

  else if ( is_special_form(form, "def-stx") )
    compile_def_stx(rls, form, vars, vals, code, fun, flags);

  else if ( is_special_form(form, "def-multi") )
    compile_def_multi(rls, form, vars, vals, code, fun, flags);

  else if ( is_special_form(form, "def-method") )
    compile_def_method(rls, form, vars, vals, code, fun, flags);

  else if ( is_special_form(form, "put") )
    compile_put(rls, form, vars, vals, code, fun, flags);

  else if ( is_special_form(form, "if") )
    compile_if(rls, form, vars, vals, code, fun, flags);

  else if ( is_special_form(form, "and") )
    compile_and(rls, form, vars, vals, code, fun, flags);

  else if ( is_special_form(form, "or") )
    compile_or(rls, form, vars, vals, code, fun, flags);

  else if ( is_special_form(form, "do") )
    compile_do(rls, form, vars, vals, code, fun, flags);

  else if ( is_special_form(form, "fn") )
    compile_fn(rls, form, vars, vals, code, fun, flags);

  else {
    int argc = form->count-1;

    while ( form->count > 0 ) {
      Expr arg = form->head;
      compile_expr(rls, arg, vars, vals, code, fun, flags);
      form = form->tail;
    }

    emit_instr(rls, code, OP_CALL, argc);
  }
}

void compile_expr(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code, Fun* fun, int* flags) {
  if ( is_sym(x) ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) ) // keywords (symbols whose names begin with ':') are treated as literals
      compile_literal(rls, x, vars, vals, code, fun, flags);

    else
      compile_reference(rls, s, vars, vals, code, fun, flags);

  } else if ( is_list(x) ) {
    List* l = as_list(x);

    if ( l->count == 0 ) // empty list is treated as a literal
      compile_literal(rls, x, vars, vals, code, fun, flags);

    else
      compile_funcall(rls, l, vars, vals, code, fun, flags);

  } else
    compile_literal(rls, x, vars, vals, code, fun, flags);
}

// External -------------------------------------------------------------------

Fun* toplevel_compile(RlState* rls, Expr x) {
  int flags = 0, sp = save_sp(rls);

  push(rls, x);

  Sym* name = mk_sym_s(rls, "&toplevel");
  Alist* vals = mk_alist_s(rls);
  Buf16* code = mk_buf16_s(rls);
  Fun* fun = mk_fun_s(rls, name, false, false);

  compile_expr(rls, x, Vm.globals, vals, code, fun, &flags);
  emit_instr(rls, code, OP_RETURN);

  Chunk* chunk = mk_chunk_s(rls, Vm.globals, vals, code);
  Method* method = mk_user_method_s(rls, fun, 0, false, chunk);

  fun_add_method(rls, fun, method);
  restore_sp(rls, sp);

  return fun;
}

Fun* compile_file(RlState* rls, char* fname) {
  int flags = 0, sp = save_sp(rls);
  Sym* name = mk_module_name_s(rls, fname);
  List* exprs = read_file_s(rls, fname);
  Alist* vals = mk_alist_s(rls);
  Buf16* code = mk_buf16_s(rls);
  Fun* fun = mk_fun_s(rls, name, false, false);

  compile_sequence(rls, exprs, Vm.globals, vals, code, fun, &flags);
  emit_instr(rls, code, OP_RETURN);

  Chunk* chunk = mk_chunk_s(rls, Vm.globals, vals, code);
  Method* method = mk_user_method_s(rls, fun, 0, false, chunk);

  fun_add_method(rls, fun, method);
  restore_sp(rls, sp);

#ifdef RASCAL_DEBUG
  // printf("\n\n=== result of compiling %s ===\n\n", fname);
  // disassemble_chunk(chunk);
#endif

  return fun;
}
