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
// compiler flags
enum {
  CF_NO_EXPR  = 0x01,
  CF_TAIL_POS = 0x02,
};

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

// miscellaneous helpers ------------------------------------------------------
Sym* mk_module_name(RlState* rls, char* fname);
Sym* mk_module_name_s(RlState* rls, char* fname);

// compile helpers ------------------------------------------------------------
Expr exec_code(RlState* rls, int argc, int flags);
Fun* compile_file(RlState* rls, char* fname);

// miscellaneous helpers ------------------------------------------------------
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
    push(rls, tag_obj(code));
    v = exec_code(rls, 0, 0);
  }

  discard_error_state(rls);

  return v;
}

// compile --------------------------------------------------------------------
// compile helpers ------------------------------------------------------------
/*
  general compile function signature:

  RlState* rls - state object
  List* form   - the form being compiled
  Env* vars    - the environment in which the expression is compiling
  Alist* vals  - constant store
  Buf16* code  - bytecode
  Fun* fun     - the function to which the compiled code will belong
 */

void emit_instr(RlState* rls, Buf16* code, OpCode op, ...);
void fill_instr(RlState* rls, Buf16* code, int offset, int val);
bool is_special_form(List* form, char* form_name);
void prepare_env(RlState* rls, char* fname, List* argl, Env* vars, int* argc, bool* va);
Ref* is_macro_call(RlState* rls, List* form, Env* vars);
Expr get_macro_expansion(RlState* rls, List* form, Env* vars, Ref* macro_ref);

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
Fun* toplevel_compile(RlState* rls, Expr x);

static void* syntax_as_obj_s(RlState* rls, char* sf, char* fn, Type* e, Expr x) {
  Type* xt = type_of(x);

  syntax_require(rls, sf, fn, xt->tag == e->tag,
                 "wanted a %s, got a %s", type_name(e), type_name(xt));

  return as_obj(x);
}

static Sym* syntax_as_sym_s(RlState* rls, char* sf, char* fn, Expr x) {
  return syntax_as_obj_s(rls, sf, fn, &SymType, x);
}

static List* syntax_as_list_s(RlState* rls, char* sf, char* fn, Expr x) {
  return syntax_as_obj_s(rls, sf, fn, &ListType, x);
}

static Fun* syntax_as_fun_s(RlState* rls, char* sf, char* fn, Expr x) {
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

// exec
bool is_falsey(Expr x) {
  return x == NONE || x == NUL || x == FALSE;
}

Expr exec_code(RlState* rls, int nargs, int flags) {
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
    [OP_ADD_METHOD]  = &&op_add_method,
    [OP_GET_LOCAL]   = &&op_get_local,
    [OP_SET_LOCAL]   = &&op_set_local,
    [OP_GET_UPVAL]   = &&op_get_upval,
    [OP_SET_UPVAL]   = &&op_set_upval,

    // jump instructions ------------------------------------------------------
    [OP_JUMP]        = &&op_jump,
    [OP_JUMP_F]      = &&op_jump_f,
    [OP_PJUMP_F]     = &&op_pjump_f,
    [OP_PJUMP_T]     = &&op_pjump_t,

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
    [OP_ISA]         = &&op_isa,
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

    // interpreter builtins ---------------------------------------------------
    [OP_APPLY]       = &&op_apply,
    [OP_COMPILE]     = &&op_compile,
    [OP_EXEC]        = &&op_exec,
    [OP_LOAD]        = &&op_load,
    [OP_ERROR]       = &&op_error,

    // environment operations -------------------------------------------------
    [OP_DEFINED]     = &&op_defined,
    [OP_LOCAL_ENV]   = &&op_local_env,
    [OP_GLOBAL_ENV]  = &&op_global_env,

    // system instructions ----------------------------------------------------
    [OP_HEAP_REPORT] = &&op_heap_report,
    [OP_STACK_REPORT]= &&op_stack_report,
    [OP_ENV_REPORT]  = &&op_env_report,
    [OP_DIS]         = &&op_dis,
  };

  (void)flags; // not used yet
  int argc=nargs, argx, argy;
  OpCode op;
  Expr x, y, z;
  Num nx, ny, nz;
  long ix, iy, iz;
  List* lx, * ly;
  Fun* fx;
  Method* method;
  Str* sx;

  // initialize
  goto do_call;

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

 op_add_method: // used to add methods to generic functions and to initialize closures.
  // leaves the method's function on the stack
  method = as_method(tos(rls));
  fun_add_method(rls, method->fun, method);
  tos(rls) = tag_obj(method->fun);
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

  // pjump instructions (peek and jump - used to implement and/or)
 op_pjump_f:
  argx = next_op(rls);
  x = tos(rls);

  if ( is_falsey(x) )
    rls->pc += argx;

  else
    pop(rls);

  goto fetch;

 op_pjump_t:
  argx = next_op(rls);

  if ( !is_falsey(x) )
    rls->pc += argx;

  else
    pop(rls);

  goto fetch;

  // closures and function calls ----------------------------------------------
 op_call:
  argc = next_op(rls);

 do_call:
  x = *stack_ref(rls, -argc-1);

  // Get the Fun and look up the appropriate Method
  fx = as_fun_s(rls, current_fn_name(rls), x);
  method = fun_get_method(fx, argc);
  require(rls, method != NULL, "%s has no method for %d arguments", fn_name(fx), argc);

  if ( is_user_method(method) )
    goto call_user_method;

  if ( method_va(method) )
    require_vargco(rls, method_name(method), method_argc(method), argc);

  else
    require_argco(rls, method_name(method), method_argc(method), argc);

  op = method->label;

  goto *labels[op];

 call_user_method:
#ifdef RASCAL_DEBUG
   if ( !method->fun->macro )
    stack_report_slice(rls, argc+1, "calling user method %s", method_name(method));
#endif

  if ( method_va(method) ) {
    require_vargco(rls, method_name(method), method_argc(method), argc);
    // collect extra args into a list
    int extra = argc - method_argc(method);
    List* rest = mk_list(rls, extra, &rls->stack[rls->sp - extra]);
    popn(rls, extra);
    push(rls, tag_obj(rest));
    argc = method_argc(method) + 1;
  } else {
    require_argco(rls, method_name(method), method_argc(method), argc);
  }

  save_call_frame(rls, argc); // save caller state
  install_method(rls, method, argc);

  goto fetch;

 op_closure:
  // Get the Fun from TOS, extract its Method, create a closure
  method = as_method(tos(rls));
  method = mk_closure(rls, method);
  tos(rls) = tag_obj(method); // make sure new Fun is visible to GC
  argc = next_op(rls);

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

 op_return:{
#ifdef RASCAL_DEBUG
    char* returning = current_fn_name(rls);
    stack_report_slice(rls, (rls->sp+1)-rls->bp, "before returning from %s", returning);
#endif

    x = pop(rls);

    if ( restore_call_frame(rls) ) { // more work to do
      push(rls, x);

#ifdef RASCAL_DEBUG
      stack_report_slice(rls, (rls->sp+1)-rls->bp, "after returning from %s", returning);
#endif
      goto fetch;
    } else { // reached toplevel, exit
      popn(rls, nargs+1); // remove caller and original arguments from stack
      return x;
    }
  }

  // builtin functions --------------------------------------------------------
  // at some hypothetical point in --------------------------------------------
  // the future these will be inlineable --------------------------------------
 op_add:
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "+", x);
  ny = as_num_s(rls, "+", y);
  nz = nx + ny;
  z = tag_num(nz);
  tos(rls) = z;           // combine push/pop
  goto fetch;

 op_sub:
  y      = pop(rls);
  x      = pop(rls);
  nx     = as_num_s(rls, "-", x);
  ny     = as_num_s(rls, "-", y);
  nz     = nx - ny;
  z      = tag_num(nz);
  tos(rls)  = z;           // combine push/pop

  goto fetch;

 op_mul:
  y      = pop(rls);
  x      = pop(rls);
  nx     = as_num_s(rls, "*", x);
  ny     = as_num_s(rls, "*", y);
  nz     = nx * ny;
  z      = tag_num(nz);
  tos(rls)  = z;           // combine push/pop
  goto fetch;

 op_div:
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "/", x);
  ny = as_num_s(rls, "/", y); require(rls, ny != 0, "division by zero");
  nz = nx / ny;
  z  = tag_num(nz);
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_rem:
  y = pop(rls);
  x = pop(rls);
  ix = as_num_s(rls, "rem", x);
  iy = as_num_s(rls, "rem", y); require(rls, ny != 0, "division by zero");
  iz = ix % iy;
  z  = tag_num(iz);
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_neq:
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "=", x);
  ny = as_num_s(rls, "=", y);
  z = nx == ny ? TRUE : FALSE;
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_nlt:
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "<", x);
  ny = as_num_s(rls, "<", y);
  z = nx < ny ? TRUE : FALSE;
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_ngt:
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, ">", x);
  ny = as_num_s(rls, ">", y);
  z = nx > ny ? TRUE : FALSE;
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_egal:
  y = pop(rls);
  x = pop(rls);
  z = egal_exps(x, y) ? TRUE : FALSE;
  tos(rls) = z;
  goto fetch;

 op_hash:
  x = pop(rls);
  ix = hash_exp(x) & XVMSK; // really this should be done consistently elsewhere
  y = tag_num(ix);
  tos(rls) = y;
  goto fetch;

 op_isa:{
    Type* t;
    y = pop(rls);
    x = pop(rls);
    t = as_type_s(rls, "isa?", y);
    z = has_type(x, t) ? TRUE : FALSE;
    tos(rls) = z;
    goto fetch;
  }

 op_type:
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
  lx = as_list_s(rls, "cons", rls->stack[rls->sp-1]);
  ly = cons(rls, rls->stack[rls->sp-2], lx);
  z  = tag_obj(ly);
  popn(rls, 2);
  tos(rls) = z;
  goto fetch;

 op_consn:
  require_argtype(rls, "cons*", &ListType, tos(rls));
  lx = cons_n(rls, argc);
  popn(rls, argc);
  tos(rls) = tag_obj(lx);
  goto fetch;

 op_head:
#ifdef RASCAL_DEBUG
  // printf("\nargument to head: ");
  // print_exp(&Outs, tos(rls));
  // printf("\n");
#endif
  x = pop(rls);
  lx = as_list_s(rls, "head", x);
  require(rls, lx->count > 0, "can't call head on empty list");
  y  = lx->head;
  tos(rls) = y;
  goto fetch;

 op_tail:

#ifdef RASCAL_DEBUG
  // stack_report_slice(rls, 2, "DEBUG - arguments to tail");
#endif

  x = pop(rls);
  lx = as_list_s(rls, "tail", x);
  require(rls, lx->count > 0, "can't call tail on empty list");
  ly = lx->tail;
  tos(rls) = tag_obj(ly);
  goto fetch;

 op_list_ref:
  x = pop(rls);
  argx = as_num_s(rls, "list-ref", x);
  x = pop(rls);
  lx = as_list_s(rls, "list-ref", x);
  require(rls, argx < (int)lx->count, "index out of bounds");
  x = list_ref(lx, argx);
  tos(rls) = x;
  goto fetch;

 op_list_len:
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
  x = pop(rls);
  sx = as_str_s(rls, "str-len", x);
  tos(rls) = tag_num(sx->count);
  goto fetch;

 op_apply:
  y = pop(rls); // arguments
  x = rpop(rls); // function to apply (removes apply from the stuck)
  ly = as_list_s(rls, "apply", y);
  require_argtype(rls, "apply", &FunType, x);
  argc = push_list(rls, ly);
  goto do_call; // jump directly to do_call (all the right variables are and stack state is correct)

 op_compile:
  y = pop(rls); // expression to compile
  fx = toplevel_compile(rls, y);
  tos(rls) = tag_obj(fx);
  goto fetch;

 op_exec:
  require_argtype(rls, "exec", &FunType, tos(rls));
  rpop(rls); // remove call to exec, leaving the function to be executed on top of the stack
  argc = 0;
  goto do_call;

 op_load:
  // compile the file and begin executing
  x = tos(rls); // needs to be preserved through GC
  sx = as_str_s(rls, "load", x);
  fx = compile_file(rls, sx->val);
  pop(rls); // remove file name from stack
  tos(rls) = tag_obj(fx); // replace `load` call with compiled code
  save_call_frame(rls, 0);
  install_method(rls, fx->method, 0);
  goto fetch;

 op_error:
  x = pop(rls);
  sx = as_str_s(rls, "error", x);
  user_error(rls, sx->val);

 op_defined:{
    y = pop(rls);
    x = pop(rls);
    Sym* n = as_sym_s(rls, "defined?", x);
    Env* e = as_env_s(rls, "defined?", y);
    Ref* r = env_resolve(rls, e, n, false);
    x = r == NULL ? FALSE : TRUE;
    tos(rls) = x;
    goto fetch;
  }

 op_local_env:{
    x = pop(rls);
    Env* e = as_env_s(rls, "local-env?", x);
    x = is_global_env(e) ? FALSE : TRUE;
    tos(rls) = x;
    goto fetch;    
  }

 op_global_env:{
    x = pop(rls);
    Env* e = as_env_s(rls, "global-env?", x);
    x = is_global_env(e) ? TRUE : FALSE;
    tos(rls) = x;
    goto fetch;
  }

 op_heap_report:
  heap_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_stack_report:
  stack_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_env_report:
  env_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_dis:
  x = pop(rls);
  fx = as_fun_s(rls, "*dis*", x);
  // Disassemble the singleton method (or first method if multimethod)
  method =  is_singleton_fn(fx) ? fx->methods : fx->methods->methods.vals[0];
  disassemble_method(method);
  tos(rls) = NUL; // dummy return value
  goto fetch;
}

// eval -----------------------------------------------------------------------
bool is_literal(Expr x) {
  if ( is_sym(x) )
    return is_keyword(as_sym(x));

  if ( is_list(x) )
    return as_list(x)->count == 0;

  return true;
}

Expr eval_exp(RlState* rls, Expr x) {
  Expr v;

  if ( is_literal(x) )
    v = x;

  else if ( is_sym(x) ) {
    Sym* s = as_sym(x);
    v = toplevel_env_get(rls, Vm.globals, s);
    require(rls, v != NONE, "unbound symbol '%s'", sym_val(s));

  } else {
    Fun* fun = toplevel_compile(rls, x);
    push(rls, tag_obj(fun));
    v = exec_code(rls, 0, 0);
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
  Expr x, v = NUL;

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
