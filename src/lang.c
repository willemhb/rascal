#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "runtime.h"
#include "opcode.h"
#include "lang.h"

// globals
Str* QuoteStr, * SetStr, * IfStr, * DoStr;

// Function prototypes
// read helpers
bool is_sym_char(int c);
bool is_num_char(int c);
int  peek(FILE *in);
char read_char(FILE *in);
void skip_space(FILE* in);
Expr read_quote(FILE* in);
Expr read_list(FILE *in);
Expr read_string(FILE* in);
Expr read_atom(FILE* in);

// read helpers
bool is_sym_char(int c) {
  return !isspace(c) && !strchr("(){}[];\"", c);
}

bool is_num_char(int c) {
  return isdigit(c) || strchr(".+-", c);
}

int peek(FILE *in) {
    int c = fgetc(in);
    ungetc(c, in);
    return c;
}

char read_char(FILE *in) {
    return fgetc(in);
}

// read helpers
void skip_space(FILE* f) {
  int c;
  
  while ( !feof(f) ) {
    c = fgetc(f);

    if ( !isspace(c) ) {
      ungetc(c, f);
      break;
    }
  }
}

Expr read_exp(FILE *in) {
  reset_token();
  skip_space(in);
  Expr x;
  int c = peek(in);

  if ( c == EOF )
    x = EOS;
  else if ( c == '\'' )
    x = read_quote(in);
  else if ( c == '(' )
    x = read_list(in);
  else if ( c == '"')
    x = read_string(in);
  else if ( is_sym_char(c) )
    x = read_atom(in);
  else if ( c == ')' )
    eval_error("dangling ')'");
  else
    eval_error("unrecognized character %c", c);

  return x;
}

Expr read_quote(FILE* in) {
  read_char(in);

  if ( feof(in) || isspace(peek(in)) )
    eval_error("invalid syntax: quoted nothing");

  Sym* hd  = mk_sym("quote"); preserve(2, tag_obj(hd), NUL);
  Expr x   = read_exp(in);    add_to_preserved(1, x);
  List* qd = mk_list(2, preserved());

  return tag_obj(qd);
}

Expr read_list(FILE *in) {
  List* out;
  read_char(in); // consume the '('
  skip_space(in);
  Expr* base = &Vm.stack[Vm.sp], x;
  int n = 0, c;

  while ( (c=peek(in)) != ')' ) {
    if ( feof(in) )
      runtime_error("unterminated list");

    x = read_exp(in);
    push(x);
    n++;
    skip_space(in);
  }

  read_char(in); // consume ')'

  out = mk_list(n, base);

  if ( n > 0 )
    popn(n);

  return tag_obj(out);
}

Expr read_string(FILE* in) {
  Str* out;

  int c;

  read_char(in); // consume opening '"'

  while ( (c=peek(in)) != '"' ) {
    if ( feof(in) )
      runtime_error("unterminated string");

    add_to_token(c); // accumulate
    read_char(in);   // advance
  }

  read_char(in); // consume terminal '"'

  out = mk_str(Token);

  return tag_obj(out);
}

Expr read_atom(FILE* in) {
  int c;
  Expr x;
  
  while ( !feof(in) && is_sym_char(c=peek(in)) ) {
    add_to_token(c); // accumulate
    read_char(in);   // consume character
  }

  assert(TOff > 0);

  if ( is_num_char(Token[0])) {
    char* end;

    Num n = strtod(Token, &end);

    if ( end[0] != '\0' ) {   // Symbol that starts with numeric character like +, -, or digit
      if ( TOff > MAX_INTERN )
        runtime_error("symbol name '%s' too long", Token);
      
      Sym* s = mk_sym(Token);
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

      Sym* s = mk_sym(Token);
      x      = tag_obj(s);
    }
  }

  return x;
}

// compile
// compile helpers
void emit_instr(Buffer* code, OpCode op, ...);
void fill_instr(Buffer* code, int offset, int val);

bool is_quote_form(List* form);
void compile_quote(List* form, Env* vars, Alist* vals, Buffer* code);
bool is_set_form(List* form);
void compile_set(List* form, Env* vars, Alist* vals, Buffer* code);
bool is_if_form(List* form);
void compile_if(List* form, Env* vars, Alist* vals, Buffer* code);
bool is_do_form(List* form);
void compile_do(List* form, Env* vars, Alist* vals, Buffer* code);
bool is_fn_form(List* form);
void compile_fn_form(List* form, Env* vars, Alist* vals, Buffer* code);

void compile_literal(Expr x, Env* vars, Alist* vals, Buffer* code);
void compile_reference(Sym* s, Env* ref, Alist* vals, Buffer* code);
void compile_funcall(List* form, Env* vars, Alist* vals, Buffer* code);
void compile_expr(Expr x, Env* vars, Alist* vals, Buffer* code);
Fun* toplevel_compile(List* form);

void emit_instr(Buffer* code, OpCode op, ...) {
  // probably not very efficient, but easy to use
  instr_t buffer[3] = { op, 0, 0 };
  va_list va;
  va_start(va, op);
  int b = 2, n = op_arity(op);

  for ( int i=0; i < n; i++ ) {
    buffer[i+1]  = va_arg(va, int);
    b           += 2;
  }

  va_end(va);

  buffer_write_n(code, (byte_t*)buffer, b);
}

void fill_instr(Buffer* code, int offset, int val) {
  ((instr_t*)code->binary.vals)[offset] = val;
}

bool is_quote_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == QuoteStr;
}

void compile_quote(List* form, Env* vars, Alist* vals, Buffer* code) {
  require_argco("quote", 1, form->count-1);

  Expr x = form->tail->head;

  compile_literal(x, vars, vals, code);
}

bool is_set_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == SetStr;
}

void compile_set(List* form, Env* vars, Alist* vals, Buffer* code) {
  require_argco("set", 2, form->count-1);
  require_argtype("set", EXP_SYM, form->tail->head);

  Sym* n = as_sym(form->tail->head);

  require(!is_keyword(n), "can't assign to keyword %s", n->val->val);
  
  int i = env_put(vars, n);

  OpCode op;

  if ( !is_local_env(vars) )
    op = OP_SET_GLOBAL;

  else if ( i < vars->arity )
    op = OP_SET_ARG;

  else
    op = OP_SET_LOCAL;
  
  compile_expr(form->tail->tail->head, vars, vals, code);
  emit_instr(code, op, i);
}

bool is_if_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == IfStr;
}

void compile_if(List* form, Env* vars, Alist* vals, Buffer* code) {
    require_argco2("if", 2, 3, form->count-1);

    Expr test = form->tail->head;
    Expr then = form->tail->tail->head;
    Expr alt  = form->count == 3 ? NUL : form->tail->tail->tail->head;

    // compile different parts of the form, saving offsets to fill in later
    compile_expr(test, vars, vals, code);
    emit_instr(code, OP_JUMP_F, 0);
    int off1 = (code->binary.count >> 1);
    compile_expr(then, vars, vals, code);
    emit_instr(code, OP_JUMP, 0);
    int off2 = (code->binary.count >> 1);
    compile_expr(alt, vars, vals, code);
    int off3 = code->binary.count >> 1;

    fill_instr(code, off1-1, off2-off1);
    fill_instr(code, off2-1, off3-off2);
}

bool is_do_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == DoStr;
}

void compile_do(List* form, Env* vars, Alist* vals, Buffer* code) {
  require_vargco("do", 2, form->count-1);

  List* xprs = form->tail;

  while ( xprs->count > 0 ) {
    Expr x = xprs->head;
    compile_expr(x, vars, vals, code);
    
    if ( xprs->count > 1 )
      emit_instr(code, OP_POP);

    xprs = xprs->tail;
  }
}

bool is_fn_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == FnStr;
}

void prepare_

void compile_fn_form(List* form, Env* vars, Alist* vals, Buffer* code) {
  
}

void compile_literal(Expr x, Env* vars, Alist* vals, Buffer* code) {
  (void)vars;
  int n = alist_push(vals, x);

  emit_instr(code, OP_GET_VALUE, n-1);
}

void compile_reference(Sym* s, Env* vars, Alist* vals, Buffer* code) {
  (void)vals;

  int n = -1;
  
  if ( is_local_env(vars) )
    n = env_resolve(vars, s);

  // either vars is global or name wasn't found
  if ( n == -1 ) {
    n = env_put(&Globals, s);
    emit_instr(code, OP_GET_GLOBAL, n);

  } else {
    if ( n < vars->arity )
      emit_instr(code, OP_GET_ARG, n);

    else
      emit_instr(code, OP_GET_LOCAL, n);
  }
}

void compile_funcall(List* form, Env* vars, Alist* vals, Buffer* code) {
  assert(form->count > 0);

  if ( is_quote_form(form) )
    compile_quote(form, vars, vals, code);

  else if ( is_set_form(form) )
    compile_set(form, vars, vals, code);

  else if ( is_if_form(form) )
    compile_if(form, vars, vals, code);

  else if ( is_do_form(form) )
    compile_do(form, vars, vals, code);

  else {
    int argc = form->count-1;

    while ( form->count > 0 ) {
      Expr arg = form->head;
      compile_expr(arg, vars, vals, code);
      form = form->tail;
    }

    emit_instr(code, OP_CALL, argc);
  }
}

void compile_expr(Expr x, Env* vars, Alist* vals, Buffer* code) {
  ExpType t = exp_type(x);
  
  if ( t == EXP_SYM ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) ) // keywords (symbols whose names begin with ':') are treated as literals
      compile_literal(x, vars, vals, code);

    else
      compile_global(s, code);

  } else if ( t == EXP_LIST ) {
    List* l = as_list(x);

    if ( l->count == 0 ) // empty list is treated as a literal
      compile_literal(x, vars, vals, code);

    else
      compile_funcall(l, vars, vals, code);

  } else
    compile_literal(x, vars, vals, code);
}

Fun* toplevel_compile(List* form) {
  preserve(3, tag_obj(form), NUL, NUL);

  Alist* vals  = mk_alist();  add_to_preserved(1, tag_obj(vals));
  Buffer* code = mk_buffer(); add_to_preserved(2, tag_obj(code));

  compile_funcall(form, &Globals, vals, code);
  emit_instr(code, OP_RETURN);

  Chunk* chunk = mk_chunk(&Globals, vals, code); add_to_preserved(1, tag_obj(chunk));
  Fun* out     = mk_user_fun(chunk);

#ifdef RASCAL_DEBUG
  disassemble(out);
#endif

  return out;
}

// exec
bool is_falsey(Expr x) {
  return x == NONE || x == NUL || x == FALSE;
}

Expr exec_code(Fun* fun) {
  void* labels[] = {
    [OP_POP]        = &&op_pop,
    [OP_GET_VALUE]  = &&op_get_value,
    [OP_GET_GLOBAL] = &&op_get_global,
    [OP_SET_GLOBAL] = &&op_set_global,
    [OP_ADD]        = &&op_add,
    [OP_SUB]        = &&op_sub,
    [OP_MUL]        = &&op_mul,
    [OP_DIV]        = &&op_div,
    [OP_EGAL]       = &&op_egal,
    [OP_TYPE]       = &&op_type,
    [OP_JUMP]       = &&op_jump,
    [OP_JUMP_F]     = &&op_jump_f,
    [OP_CALL]       = &&op_call,
    [OP_RETURN]     = &&op_return
  };

  int argc, argx;
  OpCode op;
  Expr x, y, z;
  Num nx, ny, nz;

  install_code(fun);

 fetch:
  op = next_op();

  goto *labels[op];

 op_pop: // remove TOS
  pop();

  goto fetch;

 op_get_value: // load a value from the constant store
  argx = next_op();
  x = alist_get(Vm.fn->chunk->vals, argx);
  push(x);

  goto fetch;

 op_get_global:
  argx = next_op(); // previously resolved index in global environment
  x = env_ref(&Globals, argx);
  require(x != NONE, "undefined reference");
  push(x);

  goto fetch;

 op_set_global:
  argx = next_op();
  x    = pop();
  env_refset(&Globals, argx, x);

  goto fetch;

 op_add:
  require_argco("+", 2, argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s(x);
  ny     = as_num_s(y);
  nz     = nx + ny;
  z      = tag_num(nz);
  *tos() = z;           // combine push/pop

  goto fetch;

 op_sub:
  require_argco("+", 2, argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s(x);
  ny     = as_num_s(y);
  nz     = nx - ny;
  z      = tag_num(nz);
  *tos() = z;           // combine push/pop

  goto fetch;

 op_mul:
  require_argco("*", 2, argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s(x);
  ny     = as_num_s(y);
  nz     = nx * ny;
  z      = tag_num(nz);
  *tos() = z;           // combine push/pop

  goto fetch;

 op_div:
  require_argco("/", 2, argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s(x);
  ny     = as_num_s(y); require(ny != 0, "division by zero");
  nz     = nx / ny;
  z      = tag_num(nz);
  *tos() = z;           // combine push/pop

  goto fetch;

 op_egal:
  require_argco("=", 2, argc);

  y      = pop();
  x      = pop();
  z      = egal_exps(x, y) ? TRUE : FALSE;
  *tos() = z;

  goto fetch;

 op_type:
  require_argco("type", 1, argc);

  x      = pop();
  y      = tag_obj(exp_info(x)->repr);
  *tos() = y;

  goto fetch;

 op_jump: // unconditional jumping
  argx   = next_op();
  Vm.pc += argx;

  goto fetch;

 op_jump_f: // jump if TOS is falsey
  argx   = next_op();
  x      = pop();

  if ( is_falsey(x) )
    Vm.pc += argx;

  goto fetch;

 op_call:
  argc = next_op();
  x = *stack_ref(-argc-1);
  fun = as_fun_s(x);

  assert(fun->label != OP_NOOP); // user functions not yet supported

  op = fun->label;

  goto *labels[op];

 op_return:
  x = Vm.sp ? pop() : NONE;
  reset_vm();

  return x;
}

// eval
bool is_literal(Expr x) {
  ExpType t = exp_type(x);

  return t != EXP_LIST && t != EXP_SYM;
}

Expr eval_exp(Expr x) {
  Expr v;
  
  if ( is_literal(x) )
    v = x;

  else if ( is_sym(x) ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) )
      v = x;

    else {
      v = env_get(&Globals, s);
      require(v != NONE, "unbound symbol '%s'", s->val->val);
    }
  } else {
    List* l = as_list(x);

    if ( l->count == 0 ) // empty list is self-evaluating
      v = x;

    else {
      Fun* fun;

      fun = toplevel_compile(l);
      v = exec_code(fun);
    }
  }

  return v;
}

// print
void print_exp(FILE* out, Expr x) {
  ExpTypeInfo* info = exp_info(x);

  if ( info->print_fn )
    info->print_fn(out, x);

  else
    fprintf(out, "<%s>", info->name);
}

// repl
void repl(void) {
  Expr x, v;

  for (;;) {
    if ( safepoint() )
      recover();

    else {
      fprintf(stdout, PROMPT" ");
      x = read_exp(stdin);
      v = eval_exp(x);
      fprintf(stdout, "\n>>> ");
      print_exp(stdout, v);
      fprintf(stdout, "\n\n");
    }
  }
}
