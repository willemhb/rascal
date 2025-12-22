#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "runtime.h"
#include "data.h"
#include "opcode.h"
#include "util.h"
#include "lang.h"

// globals --------------------------------------------------------------------
// Function prototypes --------------------------------------------------------
// read helpers ---------------------------------------------------------------
bool is_delim_char(int c);
bool is_sym_char(int c);
bool is_num_char(int c);
void skip_space(Port* in);
void clear_input(Port* in);
Expr read_glyph(RlState* rls, Port* in);
Expr read_quote(RlState* rls, Port* in);
Expr read_list(RlState* rls, Port *in);
Expr read_string(RlState* rls, Port* in);
Expr read_atom(RlState* rls, Port* in);

// compile helpers ------------------------------------------------------------
Expr exec_code(RlState* rls, Fun* fun, bool toplevel);
Fun* compile_file(RlState* rls, char* fname);

// read helpers ---------------------------------------------------------------
bool is_delim_char(int c) {
  return strchr("(){}[]", c);
}

bool is_sym_char(int c) {
  return !isspace(c) && !strchr("(){}[];\"", c);
}

bool is_num_char(int c) {
  return isdigit(c) || strchr(".+-", c);
}

void skip_space(Port* p) {
  int c;

  while ( !peof(p) ) {
    c = pgetc(p);

    if ( c == ';' ) {
      // skip comment to end of line
      while ( !peof(p) && (c = pgetc(p)) != '\n' )
        ;
    } else if ( !isspace(c) ) {
      pungetc(p, c);
      break;
    }
  }
}

void clear_input(Port* in) {
  int c;

  while ((c=pgetc(in)) != '\n' && c != EOF);
}

// read -----------------------------------------------------------------------
Expr read_exp(RlState* rls, Port *in) {
  reset_token(rls);
  skip_space(in);
  Expr x;
  int c = ppeekc(in);

  if ( c == EOF )
    x = EOS;
  else if ( c == '\\' )
    x = read_glyph(rls, in);
  else if ( c == '\'' )
    x = read_quote(rls, in);
  else if ( c == '(' )
    x = read_list(rls, in);
  else if ( c == '"')
    x = read_string(rls, in);
  else if ( is_sym_char(c) )
    x = read_atom(rls, in);
  else if ( c == ')' ) {
    pgetc(in); // clear dangling ')'
    eval_error(rls, "dangling ')'");
  }
  else
    eval_error(rls, "unrecognized character %c", c);

  return x;
}

Expr read_glyph(RlState* rls, Port* in) {
  pgetc(in); // consume opening slash

  if ( peof(in) || isspace(ppeekc(in)) )
    eval_error(rls, "invalid syntax: empty character");

  Glyph g; int c;

  if ( !isalpha(ppeekc(in)) ) {
    g = pgetc(in);
    c = ppeekc(in);

    require(rls, isspace(c) || is_delim_char(c), "invalid character literal");
  }

  else {
    while (!peof(in) && !isspace(c=ppeekc(in)) && !is_delim_char(c) ) {
      add_to_token(rls, c);
      pgetc(in);
    }

    if ( rls->toff == 1 )
      g = rls->token[0];

    else
      switch ( rls->token[0] ) {
        case 'n':
          if ( streq(rls->token+1, "ul") )
            g = '\0';

          else if ( streq(rls->token+1, "ewline") )
            g = '\n';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 'b':
          if ( streq(rls->token+1, "el") )
            g = '\a';

          else if ( streq(rls->token+1, "ackspace") )
            g = '\b';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 's':
          if ( streq(rls->token+1, "pace") )
            g = ' ';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 't':
          if ( streq(rls->token+1, "ab") )
            g = '\t';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 'r':
          if ( streq(rls->token+1, "eturn") )
            g = '\r';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 'f':
          if ( streq(rls->token+1, "ormfeed") )
            g = '\f';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 'v':
          if ( streq(rls->token+1, "tab") )
            g = '\v';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        default:
          eval_error(rls, "unrecognized character name \\%s", rls->token);
      }
  }

  return tag_glyph(g);
}

Expr read_quote(RlState* rls, Port* in) {
  pgetc(in); // consume opening '

  if ( peof(in) || isspace(ppeekc(in)) )
    eval_error(rls, "invalid syntax: quoted nothing");

  int sp = save_sp(rls);
  mk_sym_s(rls, "quote");
  Expr x = read_exp(rls, in); push(rls, x);
  List* qd = mk_list(rls, 2, stack_ref(rls, -2));
  restore_sp(rls, sp);

  return tag_obj(qd);
}

Expr read_list(RlState* rls, Port* in) {
  List* out;
  pgetc(in); // consume the '('
  skip_space(in);
  Expr x;
  int n = 0, c, sp = save_sp(rls);

  while ( (c=ppeekc(in)) != ')' ) {
    if ( peof(in) )
      runtime_error(rls, "unterminated list");

    x = read_exp(rls, in);
    push(rls, x);
    n++;
    skip_space(in);
  }

  pgetc(in); // consume ')'
  out = mk_list(rls, n, stack_ref(rls, -n));
  restore_sp(rls, sp); // restore stack

  return tag_obj(out);
}

Expr read_string(RlState* rls, Port* in) {
  Str* out;

  int c;

  pgetc(in); // consume opening '"'

  while ( (c=ppeekc(in)) != '"' ) {
    if ( peof(in) )
      runtime_error(rls, "unterminated string");

    add_to_token(rls, c); // accumulate
    pgetc(in);   // advance
  }

  pgetc(in); // consume terminal '"'

  out = mk_str(rls, rls->token);

  return tag_obj(out);
}

Expr read_atom(RlState* rls, Port* in) {
  int c;
  Expr x;

  while ( !peof(in) && is_sym_char(c=ppeekc(in)) ) {
    add_to_token(rls, c); // accumulate
    pgetc(in);   // consume character
  }

  assert(rls->toff > 0);

  if ( is_num_char(rls->token[0])) {
    char* end;

    Num n = strtod(rls->token, &end);

    if ( end[0] != '\0' ) {   // Symbol that starts with numeric character like +, -, or digit
      if ( rls->toff > MAX_INTERN )
        runtime_error(rls, "symbol name '%s' too long", rls->token);

      Sym* s = mk_sym(rls, rls->token);
      x      = tag_obj(s);
    } else {
      x      = tag_num(n);
    }
  } else {
    if ( strcmp(rls->token, "nul") == 0 )
      x = NUL;

    else if ( strcmp(rls->token, "none" ) == 0 )
      x = NONE;

    else if ( strcmp(rls->token, "true") == 0 )
      x = TRUE;

    else if ( strcmp(rls->token, "false") == 0 )
      x = FALSE;

    else if ( strcmp(rls->token, "<eos>" ) == 0 )
      x = EOS;

    else {
      if ( rls->toff > MAX_INTERN )
        runtime_error(rls, "symbol name '%s' too long", rls->token);

      Sym* s = mk_sym(rls, rls->token);
      x      = tag_obj(s);
    }
  }

  return x;
}

// load -----------------------------------------------------------------------
List* read_file(RlState* rls, char* fname) {
  int n = 0, sp = save_sp(rls);
  Port* in = open_port_s(rls, fname, "r");
  Expr x;

  while ( (x = read_exp(rls, in)) != EOS ) {
    push(rls, x);
    n++;
  }

  close_port(in);
  List* out = mk_list(rls, n, stack_ref(rls, -n));
  restore_sp(rls, sp); // restore stack
  return out;
}

List* read_file_s(RlState* rls, char* fname) {
  List* out = read_file(rls, fname);
  push(rls, tag_obj(out));
  return out;
}

Expr load_file(RlState* rls, char* fname) {
  Expr v; Fun* code;
  save_error_state(rls);

  if ( set_safe_point(rls) ) {
    restore_error_state(rls);
    v = NUL;
  } else {
    code = compile_file(rls, fname);
    v = exec_code(rls, code, true);
  }

  discard_error_state(rls);

  return v;
}

// compile --------------------------------------------------------------------
// compile helpers ------------------------------------------------------------
void emit_instr(RlState* rls, Buf16* code, OpCode op, ...);
void fill_instr(RlState* rls, Buf16* code, int offset, int val);

bool is_quote_form(List* form);
void compile_quote(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
bool is_def_form(List* form);
void compile_def(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
bool is_defstx_form(List* form);
void compile_defstx(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
bool is_put_form(List* form);
void compile_put(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
bool is_if_form(List* form);
void compile_if(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
bool is_do_form(List* form);
void compile_do(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
bool is_fn_form(List* form);
Fun* create_fun_from_form(RlState* rls, List* form, Env* vars, Sym* name, int* upvc_out, Env** lvars_out);
void compile_fn(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
void compile_closure(Buf16* c_code, Env* vars, Alist* vals, Buf16* code);

void compile_sequence(RlState* rls, List* exprs, Env* vars, Alist* vals, Buf16* code);
void compile_literal(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code);
void compile_reference(RlState* rls, Sym* s, Env* ref, Alist* vals, Buf16* code);
void compile_funcall(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
void compile_expr(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code);
Fun* toplevel_compile(RlState* rls, List* form);

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

bool is_quote_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && sym_val_eql(as_sym(hd), "quote");
}

void compile_quote(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  require_argco(rls, "quote", 1, form->count-1);

  Expr x = form->tail->head;

  compile_literal(rls, x, vars, vals, code);
}

bool is_def_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && sym_val_eql(as_sym(hd), "def");
}

void compile_def(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  require_argco(rls, "def", 2, form->count-1);

  Sym* n = as_sym_s(rls, "def", form->tail->head);

  require(rls, !is_keyword(n), "can't assign to keyword %s", n->val->val);

  Ref* r = env_define(rls, vars, n);
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

  compile_expr(rls, form->tail->tail->head, vars, vals, code);
  emit_instr(rls, code, op, i);
}

bool is_defstx_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && sym_val_eql(as_sym(hd), "def-stx");
}

void compile_defstx(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  require_argco(rls, "def-stx", 2, form->count-1);

  Sym* n = as_sym_s(rls, "def-stx", form->tail->head);
  require(rls, !is_keyword(n), "can't assign to keyword %s", n->val->val);

  // The second argument must be a fun form
  Expr fun_form_expr = form->tail->tail->head;
  require(rls, is_list(fun_form_expr), "def-stx: second argument must be a fun form");

  List* fun_form = as_list(fun_form_expr);
  require(rls, is_fn_form(fun_form), "def-stx: second argument must be a fun form");

  // Ensure the fun form is anonymous (not named)
  List* fun_rest = fun_form->tail;
  require(rls, !is_sym(fun_rest->head), "def-stx: fun form must be anonymous (no name)");

  // Define the macro in the environment and mark it as a macro
  Ref* r = env_define(rls, vars, n);
  r->is_macro = true;

  // Only support global macros for now
  require(rls, r->ref_type == REF_GLOBAL, "def-stx: only global macros are currently supported");

  // Create the Fun object directly without emitting instructions
  Fun* macro_fun = create_fun_from_form(rls, fun_form, vars, NULL, NULL, NULL);

  // Store the macro function immediately in the global environment
  // This makes it available for macro expansion during compilation
  Expr fun_expr = tag_obj(macro_fun);
  r->val = fun_expr;

  // Emit instruction to also set it at runtime (for consistency)
  compile_literal(rls, fun_expr, vars, vals, code);
  emit_instr(rls, code, OP_SET_GLOBAL, r->offset);
}

bool is_put_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && sym_val_eql(as_sym(hd), "put");
}

void compile_put(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  require_argco(rls, "put", 2, form->count-1);

  Sym* n = as_sym_s(rls, "put", form->tail->head);

  require(rls, !is_keyword(n), "can't assign to keyword %s", n->val->val);

  Ref* r = env_resolve(rls, vars, n, false);

  require(rls, r != NULL, "can't assign to %s before it's defined", n->val->val);

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

  compile_expr(rls, form->tail->tail->head, vars, vals, code);
  emit_instr(rls, code, op, i);
}

bool is_if_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && sym_val_eql(as_sym(hd), "if");
}

void compile_if(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  require_argco2(rls, "if", 2, 3, form->count-1);
  
  Expr test = form->tail->head;
  Expr then = form->tail->tail->head;
  Expr alt  = form->count == 3 ? NUL : form->tail->tail->tail->head;
  
  // compile different parts of the form, saving offsets to fill in later
  compile_expr(rls, test, vars, vals, code);
  emit_instr(rls, code, OP_JUMP_F, 0);
  int off1 = code->binary.count;
  compile_expr(rls, then, vars, vals, code);
  emit_instr(rls, code, OP_JUMP, 0);
  int off2 = code->binary.count;
  compile_expr(rls, alt, vars, vals, code);
  int off3 = code->binary.count;
  
  fill_instr(rls, code, off1-1, off2-off1);
  fill_instr(rls, code, off2-1, off3-off2);
}

bool is_do_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && sym_val_eql(as_sym(hd), "do");
}

void compile_do(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  require_vargco(rls, "do", 2, form->count-1);

  List* xprs = form->tail;

  compile_sequence(rls, xprs, vars, vals, code);
}

bool is_fn_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && sym_val_eql(as_sym(hd), "fun");
}

void prepare_env(RlState* rls, List* argl, Env* vars) {
  while ( argl->count > 0 ) {
    Expr x = argl->head;
    Sym* n = as_sym_s(rls, "fun", x);

    // check for & rest parameter marker
    if ( sym_val_eql(n, "&") ) {
      argl = argl->tail;
      require(rls, argl->count == 1, "& must be followed by exactly one parameter");
      n = as_sym_s(rls, "fun", argl->head);
      vars->va = true;
    }

    Ref* r = env_define(rls, vars, n);
    int  o = r->offset;

    // ensure uniqueness
    require(rls, o == vars->vars.count-1, "duplicate parameter name %s", n->val->val);

    if ( !vars->va )
      vars->arity++;

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

  if (r != NULL && r->is_macro)
    return r;

  return NULL;
}

// Expand a macro call at compile time
Expr expand_macro(RlState* rls, List* form, Env* vars, Ref* macro_ref) {
  int sp = save_sp(rls);

  // Get the macro function
  Expr macro_fun_expr;
  if (macro_ref->ref_type == REF_GLOBAL) {
    macro_fun_expr = macro_ref->val;
  } else {
    // For local macros, we'd need to get from the environment
    // For now, only support global macros
    eval_error(rls, "local macros not yet supported");
  }

  require(rls, is_fun(macro_fun_expr), "macro ref does not point to a function");

  // Push macro_fun_expr immediately to protect it from GC
  push(rls, macro_fun_expr);

  // Build argument list: (macro-fun form env arg1 arg2 ...)
  // The macro receives: whole form, environment, then individual arguments (all unevaluated)
  List* args = form->tail; // The arguments without the macro name

  // We need to build a form that when compiled will push:
  // 1. The macro fun (as a literal)
  // 2. The form (as a literal)
  // 3. The environment (as a literal)
  // 4-N. Each argument (as a literal)
  // Then call with the right argc

  // Build call_form as a list where all elements are quote expressions
  List* call_form = empty_list(rls);
  push(rls, tag_obj(call_form));  // Protect call_form as we build it

  // Build a list of quoted arguments
  // We want the result to be: (fun 'form 'env 'arg1 'arg2 'arg3)
  // So we build from right to left: start with empty, add 'arg3, 'arg2, 'arg1, 'env, 'form

  // First, iterate forward through args and quote each one, adding to call_form from the right
  List* args_copy = args;
  int nargs = args->count;

  // Collect args into array for easier reverse iteration
  Expr* arg_array = NULL;
  if (nargs > 0) {
    arg_array = (Expr*)malloc(nargs * sizeof(Expr));
    for (int i = 0; i < nargs; i++) {
      arg_array[i] = args_copy->head;
      args_copy = args_copy->tail;
    }

    // Add arguments in reverse order (rightmost first)
    for (int i = nargs - 1; i >= 0; i--) {
      // Build (quote arg)
      List* inner = cons(rls, arg_array[i], empty_list(rls));
      push(rls, tag_obj(inner));
      Sym* quote_sym = mk_sym(rls, "quote");
      push(rls, tag_obj(quote_sym));
      List* quoted_arg = cons(rls, tag_obj(quote_sym), inner);
      popn(rls, 2);

      call_form = cons(rls, tag_obj(quoted_arg), call_form);
      tos(rls) = tag_obj(call_form);
    }
    free(arg_array);
  }

  // Add environment as (quote env) where env is the environment object
  List* env_inner = cons(rls, tag_obj(vars), empty_list(rls));
  push(rls, tag_obj(env_inner));
  Sym* quote_sym1 = mk_sym(rls, "quote");
  push(rls, tag_obj(quote_sym1));
  List* quoted_env = cons(rls, tag_obj(quote_sym1), env_inner);
  popn(rls, 2);
  call_form = cons(rls, tag_obj(quoted_env), call_form);
  tos(rls) = tag_obj(call_form);

  // Add form as (quote form)
  List* form_inner = cons(rls, tag_obj(form), empty_list(rls));
  push(rls, tag_obj(form_inner));
  Sym* quote_sym2 = mk_sym(rls, "quote");
  push(rls, tag_obj(quote_sym2));
  List* quoted_form = cons(rls, tag_obj(quote_sym2), form_inner);
  popn(rls, 2);
  call_form = cons(rls, tag_obj(quoted_form), call_form);
  tos(rls) = tag_obj(call_form);

  // Add macro function at the front (it's already on stack at position sp)
  call_form = cons(rls, rls->stack[sp], call_form);
  tos(rls) = tag_obj(call_form);

  // Compile and execute the macro call
  Fun* compiled = toplevel_compile(rls, call_form);

  // Push compiled Fun to protect it during execution
  push(rls, tag_obj(compiled));

  // Execute macro with toplevel=true because we're in compilation context (no outer execution frame)
  Expr result = exec_code(rls, compiled, true);

  restore_sp(rls, sp);

  return result;
}

// Helper to create a Fun object from a fun form without emitting load instructions
// Returns the Fun, upvalue count, and environment via out parameters
Fun* create_fun_from_form(RlState* rls, List* form, Env* vars, Sym* name, int* upvc_out, Env** lvars_out) {
  require_vargco(rls, "fun", 1, form->count-1);

  int sp = save_sp(rls);
  List* rest = form->tail;

  // Skip name if present (for named form)
  if (name != NULL && is_sym(rest->head)) {
    rest = rest->tail;
    require(rls, rest->count >= 1, "fun: missing parameter list");
  }

  List* argl = as_list_s(rls, "fun", rest->head);
  List* body = rest->tail;
  Env* lvars = mk_env_s(rls, vars);
  prepare_env(rls, argl, lvars);
  Alist* lvals = mk_alist_s(rls);
  Buf16* lcode = mk_buf16_s(rls);

  // compile internal definitions first
  while ( body->count > 0 ) {
    Expr hd = body->head;

    if ( !is_list(hd) )
      break;

    List* fxpr = as_list(hd);

    if ( !is_def_form(fxpr) )
      break;

    compile_def(rls, fxpr, lvars, lvals, lcode);
    body = body->tail;
  }

  // compile remaining expressions like body of a 'do' form
  compile_sequence(rls, body, lvars, lvals, lcode);

  int ncap = lvars->ncap, upvc = lvars->upvs.count;

  if ( ncap )
    emit_instr(rls, lcode, OP_CAPTURE);

  emit_instr(rls, lcode, OP_RETURN);

  // create chunk object
  Chunk* chunk = mk_chunk_s(rls, lvars, lvals, lcode);
  Fun* fun = mk_user_fun(rls, chunk);

  // for named form, update the binding with actual function
  if (name != NULL) {
    fun->name = name;
    toplevel_env_set(rls, Vm.globals, name, tag_obj(fun));
  }

  if (upvc_out != NULL)
    *upvc_out = upvc;

  if (lvars_out != NULL)
    *lvars_out = lvars;

  restore_sp(rls, sp);

  return fun;
}

void compile_fn(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  require_vargco(rls, "fun", 1, form->count-1);

  int sp = save_sp(rls);
  List* rest = form->tail;
  Sym* name = NULL;

  // check if first arg is a symbol (named form) or list (anonymous)
  if (is_sym(rest->head)) {
    name = as_sym(rest->head);
    rest = rest->tail;
    require(rls, rest->count >= 1, "fun: missing parameter list");

    // bind name before compiling body so recursion works
    if (toplevel_env_find(rls, Vm.globals, name) == NULL) {
      toplevel_env_def(rls, Vm.globals, name, NONE);
    }
  }

  int upvc = 0;
  Env* lvars = NULL;
  Fun* fun = create_fun_from_form(rls, form, vars, name, &upvc, &lvars);

#ifdef RASCAL_DEBUG
  // disassemble(fun);
#endif

  // add instructions in caller to load resulting function object
  compile_literal(rls, tag_obj(fun), vars, vals, code);

  // add instructions to capture upvalues in calling context
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

  restore_sp(rls, sp); // restore stack
}

void compile_sequence(RlState* rls, List* xprs, Env* vars, Alist* vals, Buf16* code) {
  while ( xprs->count > 0 ) {
    Expr x = xprs->head;
    compile_expr(rls, x, vars, vals, code);

    if ( xprs->count > 1 )
      emit_instr(rls, code, OP_POP);

    xprs = xprs->tail;
  }
}

void compile_literal(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code) {
  (void)vars;

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

void compile_reference(RlState* rls, Sym* s, Env* vars, Alist* vals, Buf16* code) {
  (void)vals;

  Ref* r = env_resolve(rls, vars, s, false);

  require(rls, r != NULL, "undefined variable %s", s->val->val);

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

void compile_funcall(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  assert(form->count > 0);

  // Check for macro calls first - macros are expanded before other processing
  Ref* macro_ref = is_macro_call(rls, form, vars);
  if (macro_ref != NULL) {
    Expr expanded = expand_macro(rls, form, vars, macro_ref);
    compile_expr(rls, expanded, vars, vals, code);
    return;
  }

  if ( is_quote_form(form) )
    compile_quote(rls, form, vars, vals, code);

  else if ( is_def_form(form) ) {
    require(rls, !is_local_env(vars), "syntax error: local def in fn body");
    compile_def(rls, form, vars, vals, code);
  }

  else if ( is_defstx_form(form) ) {
    require(rls, !is_local_env(vars), "syntax error: local def-stx in fn body");
    compile_defstx(rls, form, vars, vals, code);
  }

  else if ( is_put_form(form) )
    compile_put(rls, form, vars, vals, code);

  else if ( is_if_form(form) )
    compile_if(rls, form, vars, vals, code);

  else if ( is_do_form(form) )
    compile_do(rls, form, vars, vals, code);

  else if ( is_fn_form(form) )
    compile_fn(rls, form, vars, vals, code);

  else {
    int argc = form->count-1;

    while ( form->count > 0 ) {
      Expr arg = form->head;
      compile_expr(rls, arg, vars, vals, code);
      form = form->tail;
    }

    emit_instr(rls, code, OP_CALL, argc);
  }
}

void compile_expr(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code) {
  ExpType t = exp_type(x);

  if ( t == EXP_SYM ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) ) // keywords (symbols whose names begin with ':') are treated as literals
      compile_literal(rls, x, vars, vals, code);

    else
      compile_reference(rls, s, vars, vals, code);

  } else if ( t == EXP_LIST ) {
    List* l = as_list(x);

    if ( l->count == 0 ) // empty list is treated as a literal
      compile_literal(rls, x, vars, vals, code);

    else
      compile_funcall(rls, l, vars, vals, code);

  } else
    compile_literal(rls, x, vars, vals, code);
}

Fun* toplevel_compile(RlState* rls, List* form) {
  Chunk* chunk; Fun* out; int sp = save_sp(rls);

  Alist* vals = mk_alist_s(rls);
  Buf16* code = mk_buf16_s(rls);
  compile_funcall(rls, form, Vm.globals, vals, code);
  emit_instr(rls, code, OP_RETURN);
  chunk = mk_chunk_s(rls, Vm.globals, vals, code);
  out = mk_user_fun(rls, chunk);

#ifdef RASCAL_DEBUG
  // disassemble(out);
#endif

  // Must keep Fun on stack to protect it from GC
  // We push it, then restore sp to sp+1 (keeping just the Fun)
  push(rls, tag_obj(out));
  restore_sp(rls, sp + 1);
  return out;
}

Fun* compile_file(RlState* rls, char* fname) {
  Chunk* chunk; Fun* out; int sp = save_sp(rls);
  
  List* exprs = read_file_s(rls, fname);
  Alist* vals = mk_alist_s(rls);
  Buf16* code = mk_buf16_s(rls);
  compile_sequence(rls, exprs, Vm.globals, vals, code);
  emit_instr(rls, code, OP_RETURN);
  chunk = mk_chunk_s(rls, Vm.globals, vals, code);
  out = mk_user_fun(rls, chunk);

#ifdef RASCAL_DEBUG
  // disassemble(out);
#endif

  restore_sp(rls, sp);
  return out;
}

// exec
bool is_falsey(Expr x) {
  return x == NONE || x == NUL || x == FALSE;
}

Expr exec_code(RlState* rls, Fun* fun, bool toplevel) {
  void* labels[] = {
    [OP_NOOP]        = &&op_noop,

    // stack manipulation -----------------------------------------------------
    [OP_POP]         = &&op_pop,
    [OP_RPOP]        = &&op_rpop,

    // constant loads ---------------------------------------------------------
    [OP_TRUE]        = &&op_true,
    [OP_FALSE]       = &&op_false,
    [OP_NUL]         = &&op_nul,
    [OP_ZERO]        = &&op_zero,
    [OP_ONE]         = &&op_one,

    // inlined loads ----------------------------------------------------------
    [OP_GLYPH]       = &&op_glyph,
    [OP_SMALL]       = &&op_small,

    // environment instructions -----------------------------------------------
    [OP_GET_VALUE]   = &&op_get_value,
    [OP_GET_GLOBAL]  = &&op_get_global,
    [OP_SET_GLOBAL]  = &&op_set_global,
    [OP_GET_LOCAL]   = &&op_get_local,
    [OP_SET_LOCAL]   = &&op_set_local,
    [OP_GET_UPVAL]   = &&op_get_upval,
    [OP_SET_UPVAL]   = &&op_set_upval,

    // jump instructions ------------------------------------------------------
    [OP_JUMP]        = &&op_jump,
    [OP_JUMP_F]      = &&op_jump_f,

    // function call instructions ---------------------------------------------
    [OP_CLOSURE]     = &&op_closure,
    [OP_CAPTURE]     = &&op_capture,
    [OP_CALL]        = &&op_call,
    [OP_RETURN]      = &&op_return,

    // arithmetic instructions ------------------------------------------------
    [OP_ADD]         = &&op_add,
    [OP_SUB]         = &&op_sub,
    [OP_MUL]         = &&op_mul,
    [OP_DIV]         = &&op_div,
    [OP_REM]         = &&op_rem,
    [OP_NEQ]         = &&op_neq,
    [OP_NLT]         = &&op_nlt,
    [OP_NGT]         = &&op_ngt,

    // miscellaneous builtins -------------------------------------------------
    [OP_EGAL]        = &&op_egal,
    [OP_HASH]        = &&op_hash,
    [OP_TYPE]        = &&op_type,

    // list operations --------------------------------------------------------
    [OP_LIST]        = &&op_list,
    [OP_CONS]        = &&op_cons,
    [OP_CONSN]       = &&op_consn,
    [OP_HEAD]        = &&op_head,
    [OP_TAIL]        = &&op_tail,
    [OP_LIST_REF]    = &&op_list_ref,
    [OP_LIST_LEN]    = &&op_list_len,

    // string operations ------------------------------------------------------
    [OP_STR]         = &&op_str,
    [OP_CHARS]       = &&op_chars,
    [OP_STR_REF]     = &&op_str_ref,
    [OP_STR_LEN]     = &&op_str_len,

    // system instructions ----------------------------------------------------
    [OP_HEAP_REPORT] = &&op_heap_report,
    [OP_STACK_REPORT]= &&op_stack_report,
    [OP_ENV_REPORT]  = &&op_env_report,
    [OP_DIS]         = &&op_dis,
    [OP_LOAD]        = &&op_load,
  };

  int argc, argx, argy;
  OpCode op;
  Expr x, y, z;
  Num nx, ny, nz;
  long ix, iy, iz;
  List* lx, * ly;
  Fun* fx;
  Method* method;
  Str* sx;

  // Extract the Method from the Fun
  method = fun->singleton;
  assert(method != NULL);

  if ( !toplevel )
    save_frame(rls);

  push(rls, tag_obj(fun));
  install_method(rls, method, 0);

 fetch:
  op = next_op(rls);

  goto *labels[op];

  // miscellaneous instructions -----------------------------------------------
 op_noop:
  goto fetch;

  // stack manipulation instructions ------------------------------------------
 op_pop: // remove TOS
  pop(rls);
  goto fetch;

 op_rpop: // move TOS to TOS-1, then decrement sp
  rpop(rls);
  goto fetch;

  // constant loads -----------------------------------------------------------
 op_true:
  push(rls, TRUE);
  goto fetch;

 op_false:
  push(rls, FALSE);
  goto fetch;

 op_nul:
  push(rls, NUL);
  goto fetch;

 op_zero:
  push(rls, RL_ZERO);
  goto fetch;

 op_one:
  push(rls, RL_ONE);
  goto fetch;

 op_glyph:
  argx = next_op(rls);
  x = tag_glyph(argx);
  push(rls, x);
  goto fetch;

 op_small:
  argx = (short)next_op(rls);
  x = tag_num(argx);
  push(rls, x);
  goto fetch;
  
  // value/variable instructions ----------------------------------------------
 op_get_value: // load a value from the constant store
  argx = next_op(rls);
  x = alist_get(rls->fn->chunk->vals, argx);
  push(rls, x);
  goto fetch;

 op_get_global:
  argx = next_op(rls); // previously resolved index in global environment
  x = toplevel_env_ref(rls, Vm.globals, argx);
  require(rls, x != NONE, "undefined reference");
  push(rls, x);
  goto fetch;

 op_set_global:
  argx = next_op(rls);
  x = tos(rls);
  toplevel_env_refset(rls, Vm.globals, argx, x);
  goto fetch;

 op_get_local:
  argx = next_op(rls);
  x = rls->stack[rls->bp+argx];
  push(rls, x);
  goto fetch;

 op_set_local:
  argx = next_op(rls);
  x = tos(rls);
  rls->stack[rls->bp+argx] = x;
  goto fetch;

 op_get_upval:
  argx = next_op(rls);
  x = upval_ref(rls->fn, argx);
  push(rls, x);
  goto fetch;

 op_set_upval:
  argx = next_op(rls);
  x = tos(rls);
  upval_set(rls->fn, argx, x);
  goto fetch;

  // branching instructions ---------------------------------------------------
 op_jump: // unconditional jumping
  argx = next_op(rls);
  rls->pc += argx;
  goto fetch;

 op_jump_f: // jump if TOS is falsey
  argx   = next_op(rls);
  x      = pop(rls);

  if ( is_falsey(x) )
    rls->pc += argx;

  goto fetch;

  // closures and function calls ----------------------------------------------
 op_call:
  argc = next_op(rls);
  x    = *stack_ref(rls, -argc-1);

  // Get the Fun and look up the appropriate Method
  fx = as_fun_s(rls, rls->fn->name->val->val, x);
  method = fun_lookup(fx, argc);
  require(rls, method != NULL, "%s has no method for %d arguments",
          fx->name->val->val, argc);

  if ( is_user_method(method) )
    goto call_user_method;

  op = method->label;

  goto *labels[op];

 call_user_method:
  if ( method_va(method) ) {
    require_vargco(rls, method->name->val->val, method_argc(method), argc);
    // collect extra args into a list
    int extra = argc - method_argc(method);
    List* rest = mk_list_s(rls, extra, &rls->stack[rls->sp - extra]);
    popn(rls, extra);
    push(rls, tag_obj(rest));
    argc = method_argc(method) + 1;
  } else {
    require_argco(rls, method->name->val->val, method_argc(method), argc);
  }
  save_frame(rls); // save caller state
  install_method(rls, method, argc);

  goto fetch;

 op_closure:
  // Get the Fun from TOS, extract its Method, create a closure
  fx = as_fun(tos(rls));
  method = mk_closure(rls, fx->singleton);
  // Create a new Fun with the closure Method
  fx = mk_fun(rls, fx->name, method);
  tos(rls) = tag_obj(fx);     // make sure new Fun is visible to GC
  argc  = next_op(rls);

  for ( int i=0; i < argc; i++ ) {
    argx = next_op(rls);
    argy = next_op(rls);

    if ( argx == 0 ) // nonlocal
      method->upvs.vals[i] = rls->fn->upvs.vals[argy];

    else
      method->upvs.vals[i] = get_upv(rls, rls->stack+rls->bp+argy);
  }

  goto fetch;

  // emmitted before a frame with local upvalues returns
 op_capture:
  close_upvs(rls, rls->stack+rls->bp);

  goto fetch;

 op_return:
  x    = rls->sp > rls->fp ? pop(rls) : NUL;

  if ( rls->fp == 0 ) { // no calling frame, exit
    reset_vm(rls);
    return x;
  }

  argx  = rls->bp;       // adjust stack to here after restore

  restore_frame(rls);

  rls->sp = argx;
  tos(rls) = x;

  goto fetch;

  // builtin functions --------------------------------------------------------
  // at some hypothetical point in --------------------------------------------
  // the future these will be inlineable --------------------------------------
 op_add:
  require_argco(rls, "+", 2, argc);
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "+", x);
  ny = as_num_s(rls, "+", y);
  nz = nx + ny;
  z = tag_num(nz);
  tos(rls) = z;           // combine push/pop

  goto fetch;

 op_sub:
  require_argco(rls, "-", 2, argc);

  y      = pop(rls);
  x      = pop(rls);
  nx     = as_num_s(rls, "-", x);
  ny     = as_num_s(rls, "-", y);
  nz     = nx - ny;
  z      = tag_num(nz);
  tos(rls)  = z;           // combine push/pop

  goto fetch;

 op_mul:
  require_argco(rls, "*", 2, argc);

  y      = pop(rls);
  x      = pop(rls);
  nx     = as_num_s(rls, "*", x);
  ny     = as_num_s(rls, "*", y);
  nz     = nx * ny;
  z      = tag_num(nz);
  tos(rls)  = z;           // combine push/pop
  goto fetch;

 op_div:
  require_argco(rls,"/", 2, argc);
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "/", x);
  ny = as_num_s(rls, "/", y); require(rls, ny != 0, "division by zero");
  nz = nx / ny;
  z  = tag_num(nz);
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_rem:
  require_argco(rls,"rem", 2, argc);
  y = pop(rls);
  x = pop(rls);
  ix = as_num_s(rls, "rem", x);
  iy = as_num_s(rls, "rem", y); require(rls, ny != 0, "division by zero");
  iz = ix % iy;
  z  = tag_num(iz);
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_neq:
  require_argco(rls, "=", 2, argc);
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "=", x);
  ny = as_num_s(rls, "=", y);
  z = nx == ny ? TRUE : FALSE;
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_nlt:
  require_argco(rls, "<", 2, argc);
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "<", x);
  ny = as_num_s(rls, "<", y);
  z = nx < ny ? TRUE : FALSE;
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_ngt:
  require_argco(rls, ">", 2, argc);
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, ">", x);
  ny = as_num_s(rls, ">", y);
  z = nx > ny ? TRUE : FALSE;
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_egal:
  require_argco(rls, "=?", 2, argc);
  y = pop(rls);
  x = pop(rls);
  z = egal_exps(x, y) ? TRUE : FALSE;
  tos(rls) = z;
  goto fetch;

 op_hash:
  require_argco(rls, "hash", 1, argc);
  x = pop(rls);
  ix = hash_exp(x) & XVMSK; // really this should be done consistently elsewhere
  y = tag_num(ix);
  tos(rls) = y;
  goto fetch;

 op_type:
  require_argco(rls, "type", 1, argc);
  x = pop(rls);
  y = tag_obj(type_of(x));
  tos(rls) = y;
  goto fetch;

 op_list:
  lx = mk_list(rls, argc, &rls->stack[rls->sp-argc]);
  popn(rls, argc);
  tos(rls) = tag_obj(lx);
  goto fetch;

 op_cons:
  require_argco(rls,"cons", 2, argc);
  lx = as_list_s(rls, "cons", rls->stack[rls->sp-1]);
  ly = cons(rls, rls->stack[rls->sp-2], lx);
  z  = tag_obj(ly);
  popn(rls, 2);
  tos(rls) = z;
  goto fetch;

 op_consn:
  require_vargco(rls, "cons*", 2, argc);
  require_argtype(rls, "cons*", &ListType, tos(rls));
  lx = cons_n(rls, argc);
  popn(rls, argc);
  tos(rls) = tag_obj(lx);
  goto fetch;

 op_head:
  require_argco(rls,"head", 1, argc);
  x = pop(rls);
  lx = as_list_s(rls, "head", x);
  require(rls, lx->count > 0, "can't call head on empty list");
  y  = lx->head;
  tos(rls) = y;
  goto fetch;

 op_tail:
  require_argco(rls,"tail", 1, argc);
  x = pop(rls);
  lx = as_list_s(rls, "tail", x);
  require(rls, lx->count > 0, "can't call tail on empty list");
  ly = lx->tail;
  tos(rls) = tag_obj(ly);

  goto fetch;

 op_list_ref:
  require_argco(rls,"list-ref", 2, argc);
  x = pop(rls);
  argx = as_num_s(rls, "list-ref", x);
  x = pop(rls);
  lx = as_list_s(rls, "list-ref", x);
  require(rls, argx < (int)lx->count, "index out of bounds");
  x = list_ref(lx, argx);
  tos(rls) = x;
  goto fetch;

 op_list_len:
  require_argco(rls,"list-len", 1, argc);
  x = pop(rls);
  lx = as_list_s(rls, "list-len", x);
  tos(rls) = tag_num(lx->count);
  goto fetch;

 op_str:{ // string constructor (two modes, accepts characters or list of characters)
  if ( argc == 1 && is_list(tos(rls)) ) {
    x = pop(rls);
    lx = as_list(x);
    argc = lx->count;

    // unpack onto stack (type verified later)
    while ( lx->count > 0 ) {
      push(rls, lx->head);
      lx = lx->tail;
    }
  }

  // create a buffer to hold the characters
  char buf[argc+1] = {};

  for ( int i=argc-1; i >=0; i-- ) {
    x = pop(rls);
    buf[i] = as_glyph_s(rls, "str", x);
  }

  sx = mk_str(rls, buf);
  tos(rls) = tag_obj(sx);
  goto fetch;
  }

 op_chars:
  require_argco(rls,"chars", 1, argc);
  x = pop(rls);
  sx = as_str_s(rls, "chars", x);
  argc = sx->count;

  for ( int i=0; i<argc; i++ ) {
    argx = sx->val[i];
    x = tag_glyph(argx);
    push(rls, x);
  }

  goto op_list;

 op_str_ref:
  require_argco(rls,"str-ref", 2, argc);
  x = pop(rls);
  argx = as_num_s(rls, "str-ref", x);
  y = pop(rls);
  sx = as_str_s(rls, "str-ref", y);
  require(rls, argx < (int)sx->count, "index out of bounds");
  argy = sx->val[argx];
  x = tag_glyph(argy);
  tos(rls) = x;
  goto fetch;

 op_str_len:
  require_argco(rls,"str-len", 1, argc);
  x = pop(rls);
  sx = as_str_s(rls, "str-len", x);
  tos(rls) = tag_num(sx->count);
  goto fetch;

 op_heap_report:
  require_argco(rls,"*heap-report*", 0, argc);
  heap_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_stack_report:
  require_argco(rls,"*stack-report*", 0, argc);
  stack_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_env_report:
  require_argco(rls, "*env-report*", 0, argc);
  env_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_dis:
  require_argco(rls, "*dis*", 1, argc);
  x = pop(rls);
  fx = as_fun_s(rls, "*dis*", x);
  // Disassemble the singleton method (or first method if multimethod)
  method = fx->singleton ? fx->singleton : fx->methods->methods.vals[0];
  disassemble(method);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_load:
  // compile the file and begin executing
  require_argco(rls, "load", 1, argc);
  x = pop(rls);
  sx = as_str_s(rls, "load", x);
  fx = compile_file(rls, sx->val);
  save_frame(rls);
  install_method(rls, fx->singleton, 0);
  goto fetch;
}

// eval -----------------------------------------------------------------------
bool is_literal(Expr x) {
  ExpType t = exp_type(x);

  return t != EXP_LIST && t != EXP_SYM;
}

Expr eval_exp(RlState* rls, Expr x) {
  Expr v;

  if ( is_literal(x) )
    v = x;

  else if ( is_sym(x) ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) )
      v = x;

    else {
      v = toplevel_env_get(rls, Vm.globals, s);
      require(rls, v != NONE, "unbound symbol '%s'", s->val->val);
    }
  } else {
    List* l = as_list(x);

    if ( l->count == 0 ) // empty list is self-evaluating
      v = x;

    else {
      Fun* fun;

      fun = toplevel_compile(rls, l);
      v = exec_code(rls, fun, true);
    }
  }

  return v;
}

// print ----------------------------------------------------------------------
void print_exp(Port* out, Expr x) {
  Type* info = type_of(x);

  if ( info->print_fn )
    info->print_fn(out, x);

  else
    pprintf(out, "<%s>", type_name(info));
}

// repl -----------------------------------------------------------------------
void repl(RlState* rls) {
  Expr x, v;

  // create a catch point
  save_error_state(rls);

  for (;;) {
    fprintf(stdout, PROMPT" ");

    // set safe point for read (so that errors can be handled properly)
    if ( set_safe_point(rls) ) {
      restore_error_state(rls);
      clear_input(&Ins);
      goto next_line;
    } else {
      x = read_exp(rls, &Ins);
    }

    if ( set_safe_point(rls) ) {
      restore_error_state(rls);
      goto next_line;
    } else {
      v = eval_exp(rls, x);
      pprintf(&Outs, "\n>>> ");
      print_exp(&Outs, v);
    }

  next_line:
    pprintf(&Outs, "\n\n");
  }

  // discard catch point
  discard_error_state(rls);
}
