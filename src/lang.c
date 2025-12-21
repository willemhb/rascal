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
Str* QuoteStr, *DefStr, * PutStr, * IfStr, * DoStr, * FnStr;

// Function prototypes --------------------------------------------------------
// read helpers ---------------------------------------------------------------
bool is_delim_char(int c);
bool is_sym_char(int c);
bool is_num_char(int c);
void skip_space(Port* in);
Expr read_glyph(RlState* rls, Port* in);
Expr read_quote(RlState* rls, Port* in);
Expr read_list(RlState* rls, Port *in);
Expr read_string(RlState* rls, Port* in);
Expr read_atom(RlState* rls, Port* in);

// compile helpers ------------------------------------------------------------
Expr exec_code(RlState* rls, Fun* code, bool toplevel);
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

// read -----------------------------------------------------------------------
Expr read_exp(RlState* rls, Port *in) {
  reset_token();
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
  else if ( c == ')' )
    eval_error("dangling ')'");
  else
    eval_error("unrecognized character %c", c);

  return x;
}

Expr read_glyph(RlState* rls, Port* in) {
  (void)rls; // will be used later, suppressing warning
  pgetc(in); // consume opening slash

  if ( peof(in) || isspace(ppeekc(in)) )
    eval_error("invalid syntax: empty character");

  Glyph g; int c;

  if ( !isalpha(ppeekc(in)) ) {
    g = pgetc(in);
    c = ppeekc(in);

    require(isspace(c) || is_delim_char(c), "invalid character literal");
  }

  else {
    while (!peof(in) && !isspace(c=ppeekc(in)) && !is_delim_char(c) ) {
      add_to_token(c);
      pgetc(in);
    }

    if ( TOff == 1 )
      g = Token[0];

    else
      switch ( Token[0] ) {
        case 'n':
          if ( streq(Token+1, "ul") )
            g = '\0';
          
          else if ( streq(Token+1, "ewline") )
            g = '\n';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 'b':
          if ( streq(Token+1, "el") )
            g = '\a';

          else if ( streq(Token+1, "ackspace") )
            g = '\b';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 's':
          if ( streq(Token+1, "pace") )
            g = ' ';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 't':
          if ( streq(Token+1, "ab") )
            g = '\t';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 'r':
          if ( streq(Token+1, "eturn") )
            g = '\r';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 'f':
          if ( streq(Token+1, "ormfeed") )
            g = '\f';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 'v':
          if ( streq(Token+1, "tab") )
            g = '\v';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        default:
          eval_error("unrecognized character name \\%s", Token);
      }
  }

  return tag_glyph(g);
}

Expr read_quote(RlState* rls, Port* in) {
  pgetc(in); // consume opening '

  if ( peof(in) || isspace(ppeekc(in)) )
    eval_error("invalid syntax: quoted nothing");

  Sym* hd  = mk_sym(rls, "quote"); preserve(2, tag_obj(hd), NUL);
  Expr x   = read_exp(rls, in);    add_to_preserved(1, x);
  List* qd = mk_list(rls, 2, preserved());

  return tag_obj(qd);
}

Expr read_list(RlState* rls, Port* in) {
  List* out;
  pgetc(in); // consume the '('
  skip_space(in);
  Expr* base = &rls->stack[rls->sp], x;
  int n = 0, c;

  while ( (c=ppeekc(in)) != ')' ) {
    if ( peof(in) )
      runtime_error("unterminated list");

    x = read_exp(rls, in);
    push(rls, x);
    n++;
    skip_space(in);
  }

  pgetc(in); // consume ')'

  out = mk_list(rls, n, base);

  if ( n > 0 )
    popn(rls, n);

  return tag_obj(out);
}

Expr read_string(RlState* rls, Port* in) {
  Str* out;

  int c;

  pgetc(in); // consume opening '"'

  while ( (c=ppeekc(in)) != '"' ) {
    if ( peof(in) )
      runtime_error("unterminated string");

    add_to_token(c); // accumulate
    pgetc(in);   // advance
  }

  pgetc(in); // consume terminal '"'

  out = mk_str(rls, Token);

  return tag_obj(out);
}

Expr read_atom(RlState* rls, Port* in) {
  (void)rls; // will be used after refactor
  int c;
  Expr x;
  
  while ( !peof(in) && is_sym_char(c=ppeekc(in)) ) {
    add_to_token(c); // accumulate
    pgetc(in);   // consume character
  }

  assert(TOff > 0);

  if ( is_num_char(Token[0])) {
    char* end;

    Num n = strtod(Token, &end);

    if ( end[0] != '\0' ) {   // Symbol that starts with numeric character like +, -, or digit
      if ( TOff > MAX_INTERN )
        runtime_error("symbol name '%s' too long", Token);

      Sym* s = mk_sym(rls, Token);
      x      = tag_obj(s);
    } else {
      x      = tag_num(n);
    }
  } else {
    if ( strcmp(Token, "nul") == 0 )
      x = NUL;

    else if ( strcmp(Token, "none" ) == 0 )
      x = NONE;

    else if ( strcmp(Token, "true") == 0 )
      x = TRUE;

    else if ( strcmp(Token, "false") == 0 )
      x = FALSE;

    else if ( strcmp(Token, "<eos>" ) == 0 )
      x = EOS;

    else {
      if ( TOff > MAX_INTERN )
        runtime_error("symbol name '%s' too long", Token);

      Sym* s = mk_sym(rls, Token);
      x      = tag_obj(s);
    }
  }

  return x;
}

// load -----------------------------------------------------------------------
List* read_file(RlState* rls, char* fname) {
  Port* in = open_port(rls, fname, "r");
  preserve(1, tag_obj(in));  // protect port from GC

  Expr* base = &rls->stack[rls->sp];
  Expr x;
  int n = 0;

  while ( (x = read_exp(rls, in)) != EOS ) {
    push(rls, x);
    n++;
  }

  close_port(in);

  List* out = mk_list(rls, n, base);

  if ( n > 0 )
    popn(rls, n);

  return out;
}

Expr load_file(RlState* rls, char* fname) {
  Fun* code = compile_file(rls, fname);
  Expr v = exec_code(rls, code, true);

  return v;
}

// compile --------------------------------------------------------------------
// compile helpers ------------------------------------------------------------
void emit_instr(Buf16* code, OpCode op, ...);
void fill_instr(Buf16* code, int offset, int val);

bool is_quote_form(List* form);
void compile_quote(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
bool is_def_form(List* form);
void compile_def(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
bool is_put_form(List* form);
void compile_put(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
bool is_if_form(List* form);
void compile_if(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
bool is_do_form(List* form);
void compile_do(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
bool is_fn_form(List* form);
void compile_fn(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
void compile_closure(Buf16* c_code, Env* vars, Alist* vals, Buf16* code);

void compile_sequence(RlState* rls, List* exprs, Env* vars, Alist* vals, Buf16* code);
void compile_literal(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code);
void compile_reference(Sym* s, Env* ref, Alist* vals, Buf16* code);
void compile_funcall(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code);
void compile_expr(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code);
Fun* toplevel_compile(RlState* rls, List* form);

void emit_instr(Buf16* code, OpCode op, ...) {
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
}

void fill_instr(Buf16* code, int offset, int val) {
  ((instr_t*)code->binary.vals)[offset] = val;
}

bool is_quote_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == QuoteStr;
}

void compile_quote(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  require_argco("quote", 1, form->count-1);

  Expr x = form->tail->head;

  compile_literal(rls, x, vars, vals, code);
}

bool is_def_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == DefStr;
}

void compile_def(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  require_argco("def", 2, form->count-1);

  Sym* n = as_sym_s("def", form->tail->head);

  require(!is_keyword(n), "can't assign to keyword %s", n->val->val);

  Ref* r = env_define(vars, n);
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
  emit_instr(code, op, i);
}

bool is_put_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == PutStr;
}

void compile_put(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
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
      break;

    default:
      unreachable();
  }

  compile_expr(rls, form->tail->tail->head, vars, vals, code);
  emit_instr(code, op, i);
}

bool is_if_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == IfStr;
}

void compile_if(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
    require_argco2("if", 2, 3, form->count-1);

    Expr test = form->tail->head;
    Expr then = form->tail->tail->head;
    Expr alt  = form->count == 3 ? NUL : form->tail->tail->tail->head;

    // compile different parts of the form, saving offsets to fill in later
    compile_expr(rls, test, vars, vals, code);
    emit_instr(code, OP_JUMP_F, 0);
    int off1 = code->binary.count;
    compile_expr(rls, then, vars, vals, code);
    emit_instr(code, OP_JUMP, 0);
    int off2 = code->binary.count;
    compile_expr(rls, alt, vars, vals, code);
    int off3 = code->binary.count;

    fill_instr(code, off1-1, off2-off1);
    fill_instr(code, off2-1, off3-off2);
}

bool is_do_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == DoStr;
}

void compile_do(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  require_vargco("do", 2, form->count-1);

  List* xprs = form->tail;

  compile_sequence(rls, xprs, vars, vals, code);
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

void compile_fn(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  require_vargco("fn", 1, form->count-1);

  List* argl  = as_list_s("fn", form->tail->head);
  List* body  = form->tail->tail;
  Env*  lvars = mk_env(rls, vars);

  preserve(3, tag_obj(lvars), NUL, NUL);
  prepare_env(argl, lvars);

  Alist* lvals = mk_alist(rls); add_to_preserved(1, tag_obj(lvals));
  Buf16* lcode = mk_buf16(rls); add_to_preserved(2, tag_obj(lcode));

  // compile internal definitions first
  // otherwise their values will get lost in the stack
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
    emit_instr(lcode, OP_CAPTURE);

  emit_instr(lcode, OP_RETURN);

  // create chunk object
  Chunk* chunk = mk_chunk(rls, lvars, lvals, lcode); add_to_preserved(0, tag_obj(chunk));
  Fun* fun     = mk_user_fun(rls, chunk);

#ifdef RASCAL_DEBUG
  // disassemble(fun);
#endif

  // add instructions in caller to load resulting function object
  compile_literal(rls, tag_obj(fun), vars, vals, code);

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

void compile_sequence(RlState* rls, List* xprs, Env* vars, Alist* vals, Buf16* code) {
  while ( xprs->count > 0 ) {
    Expr x = xprs->head;
    compile_expr(rls, x, vars, vals, code);

    if ( xprs->count > 1 )
      emit_instr(code, OP_POP);

    xprs = xprs->tail;
  }
}

void compile_literal(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code) {
  (void)vars;

  if ( x == TRUE ) {
    emit_instr(code, OP_TRUE);
  } else if ( x == FALSE ) {
    emit_instr(code, OP_FALSE);
  } else if ( x == NUL ) {
    emit_instr(code, OP_NUL);
  } else {
    int n = alist_push(rls, vals, x);

    emit_instr(code, OP_GET_VALUE, n-1);
  }
}

void compile_reference(Sym* s, Env* vars, Alist* vals, Buf16* code) {
  (void)vals;

  Ref* r = env_resolve(vars, s, false);

  require(r != NULL, "undefined variable %s", s->val->val);

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

  emit_instr(code, op, i);
}

void compile_funcall(RlState* rls, List* form, Env* vars, Alist* vals, Buf16* code) {
  assert(form->count > 0);

  if ( is_quote_form(form) )
    compile_quote(rls, form, vars, vals, code);

  else if ( is_def_form(form) ) {
    require(!is_local_env(vars), "syntax error: local def in fn body");
    compile_def(rls, form, vars, vals, code);
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

    emit_instr(code, OP_CALL, argc);
  }
}

void compile_expr(RlState* rls, Expr x, Env* vars, Alist* vals, Buf16* code) {
  ExpType t = exp_type(x);

  if ( t == EXP_SYM ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) ) // keywords (symbols whose names begin with ':') are treated as literals
      compile_literal(rls, x, vars, vals, code);

    else
      compile_reference(s, vars, vals, code);

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
  preserve(3, tag_obj(form), NUL, NUL);

  Alist* vals  = mk_alist(rls); add_to_preserved(1, tag_obj(vals));
  Buf16* code = mk_buf16(rls); add_to_preserved(2, tag_obj(code));

  compile_funcall(rls, form, &Globals, vals, code);
  emit_instr(code, OP_RETURN);

  Chunk* chunk = mk_chunk(rls, &Globals, vals, code); add_to_preserved(0, tag_obj(chunk)); // reuse saved slot
  Fun* out     = mk_user_fun(rls, chunk);

#ifdef RASCAL_DEBUG
  disassemble(out);
#endif

  return out;
}

Fun* compile_file(RlState* rls, char* fname) {
  List* exprs = read_file(rls, fname);
  preserve(4, tag_obj(exprs), NUL, NUL, NUL);
  Alist* vals = mk_alist(rls); add_to_preserved(1, tag_obj(vals));
  Buf16* code = mk_buf16(rls); add_to_preserved(2, tag_obj(code));

  compile_sequence(rls, exprs, &Globals, vals, code);
  emit_instr(code, OP_RETURN);

  Chunk* chunk = mk_chunk(rls, &Globals, vals, code); add_to_preserved(0, tag_obj(chunk));
  Fun* out = mk_user_fun(rls, chunk);

#ifdef RASCAL_DEBUG
  disassemble(out);
#endif

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

    // miscellaneous builtins -------------------------------------------------
    [OP_EGAL]        = &&op_egal,
    [OP_TYPE]        = &&op_type,

    // list operations --------------------------------------------------------
    [OP_LIST]        = &&op_list,
    [OP_CONS]        = &&op_cons,
    [OP_HEAD]        = &&op_head,
    [OP_TAIL]        = &&op_tail,
    [OP_LIST_REF]    = &&op_list_ref,
    [OP_LIST_LEN]    = &&op_list_len,

    // string operations ------------------------------------------------------
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
  List* lx, * ly;
  Fun* fx;
  Str* sx;

  if ( !toplevel )
    save_frame(rls);

  push(rls, tag_obj(fun));
  install_fun(rls, fun, 0);

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
  
  // value/variable instructions ----------------------------------------------
 op_get_value: // load a value from the constant store
  argx = next_op(rls);
  x = alist_get(rls->fn->chunk->vals, argx);
  push(rls, x);
  goto fetch;

 op_get_global:
  argx = next_op(rls); // previously resolved index in global environment
  x = toplevel_env_ref(&Globals, argx);
  require(x != NONE, "undefined reference");
  push(rls, x);
  goto fetch;

 op_set_global:
  argx = next_op(rls);
  x = tos(rls);
  toplevel_env_refset(&Globals, argx, x);
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
  fun  = as_fun_s(rls->fn->name->val->val, x);

  if ( is_user_fn(fun) )
    goto call_user_fn;

  op = fun->label;

  goto *labels[op];

 call_user_fn:
  require_argco("fn", user_fn_argc(fun), argc);
  save_frame(rls); // save caller state
  install_fun(rls, fun, argc);

  goto fetch;

 op_closure:
  fun   = as_fun(tos(rls));
  fun   = mk_closure(rls, fun);
  tos(rls) = tag_obj(fun);     // make sure new closure is visible to GC
  argc  = next_op(rls);

  for ( int i=0; i < argc; i++ ) {
    argx = next_op(rls);
    argy = next_op(rls);

    if ( argx == 0 ) // nonlocal
      fun->upvs.vals[i] = rls->fn->upvs.vals[argy];

    else
      fun->upvs.vals[i] = get_upv(rls, rls->stack+rls->bp+argy);
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
  require_argco("+", 2, argc);

  y      = pop(rls);
  x      = pop(rls);
  nx     = as_num_s("+", x);
  ny     = as_num_s("+", y);
  nz     = nx + ny;
  z      = tag_num(nz);
  tos(rls)  = z;           // combine push/pop

  goto fetch;

 op_sub:
  require_argco("-", 2, argc);

  y      = pop(rls);
  x      = pop(rls);
  nx     = as_num_s("-", x);
  ny     = as_num_s("-", y);
  nz     = nx - ny;
  z      = tag_num(nz);
  tos(rls)  = z;           // combine push/pop

  goto fetch;

 op_mul:
  require_argco("*", 2, argc);

  y      = pop(rls);
  x      = pop(rls);
  nx     = as_num_s("*", x);
  ny     = as_num_s("*", y);
  nz     = nx * ny;
  z      = tag_num(nz);
  tos(rls)  = z;           // combine push/pop

  goto fetch;

 op_div:
  require_argco("/", 2, argc);

  y      = pop(rls);
  x      = pop(rls);
  nx     = as_num_s("/", x);
  ny     = as_num_s("/", y); require(ny != 0, "division by zero");
  nz     = nx / ny;
  z      = tag_num(nz);
  tos(rls)  = z;           // combine push/pop

  goto fetch;

 op_egal:
  require_argco("=", 2, argc);

  y      = pop(rls);
  x      = pop(rls);
  z      = egal_exps(x, y) ? TRUE : FALSE;
  tos(rls)  = z;

  goto fetch;

 op_type:
  require_argco("type", 1, argc);

  x = pop(rls);
  y = tag_obj(exp_info(x)->repr);
  tos(rls) = y;

  goto fetch;

 op_list:
  lx = mk_list(rls, argc, &rls->stack[rls->sp-argc]);
  popn(rls, argc);
  tos(rls) = tag_obj(lx);

  goto fetch;

 op_cons:
  require_argco("cons", 2, argc);
  lx = as_list_s("cons", rls->stack[rls->sp-1]);
  ly = cons(rls, rls->stack[rls->sp-2], lx);
  z  = tag_obj(ly);
  popn(rls, 2);
  tos(rls) = z;
  goto fetch;

 op_head:
  require_argco("head", 1, argc);
  x = pop(rls);
  lx = as_list_s("head", x);
  require(lx->count > 0, "can't call head on empty list");
  y  = lx->head;
  tos(rls) = y;
  goto fetch;

 op_tail:
  require_argco("tail", 1, argc);
  x = pop(rls);
  lx = as_list_s("tail", x);
  require(lx->count > 0, "can't call tail on empty list");
  ly = lx->tail;
  tos(rls) = tag_obj(ly);

  goto fetch;

 op_list_ref:
  require_argco("list-ref", 2, argc);
  x = pop(rls);
  argx = as_num_s("list-ref", x);
  x = pop(rls);
  lx = as_list_s("list-ref", x);
  require(argx < (int)lx->count, "index out of bounds");
  x = list_ref(lx, argx);
  tos(rls) = x;

  goto fetch;

 op_list_len:
  require_argco("list-len", 1, argc);
  x = pop(rls);
  lx = as_list_s("list-len", x);
  tos(rls) = tag_num(lx->count);

  goto fetch;

 op_str_ref:
  require_argco("str-ref", 2, argc);
  x = pop(rls);
  argx = as_num_s("str-ref", x);
  y = pop(rls);
  sx = as_str_s("str-ref", y);
  require(argx < (int)sx->count, "index out of bounds");
  argy = sx->val[argx];
  x = tag_glyph(argy);
  tos(rls) = x;
  goto fetch;

 op_str_len:
  require_argco("str-len", 1, argc);
  x = pop(rls);
  sx = as_str_s("str-len", x);
  tos(rls) = tag_num(sx->count);
  goto fetch;

 op_heap_report:
  require_argco("*heap-report*", 0, argc);
  heap_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_stack_report:
  require_argco("*stack-report*", 0, argc);
  stack_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_env_report:
  require_argco("*env-report*", 0, argc);
  stack_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_dis:
  require_argco("*dis*", 1, argc);
  x = pop(rls);
  fx = as_fun_s("*dis*", x);
  disassemble(fx);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_load:
  // compile the file and begin executing
  require_argco("load", 1, argc);
  x = pop(rls);
  sx = as_str_s("load", x);
  fx = compile_file(rls, sx->val);
  save_frame(rls);
  install_fun(rls, fx, 0);
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
      v = toplevel_env_get(&Globals, s);
      require(v != NONE, "unbound symbol '%s'", s->val->val);
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
  ExpTypeInfo* info = exp_info(x);

  if ( info->print_fn )
    info->print_fn(out, x);

  else
    pprintf(out, "<%s>", info->name);
}

// repl -----------------------------------------------------------------------
void repl(RlState* rls) {
  Expr x, v;

  // create a catch point
  save_ctx(rls);

  for (;;) {
    if ( safepoint() )
      recover();

    else {
      fprintf(stdout, PROMPT" ");
      x = read_exp(rls, &Ins);
      v = eval_exp(rls, x);
      fprintf(stdout, "\n>>> ");
      print_exp(&Outs, v);
      fprintf(stdout, "\n\n");
    }
  }

  // discard catch point
  discard_ctx();
}
