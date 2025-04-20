#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "runtime.h"
#include "lang.h"

// Function prototypes
// read helpers
bool is_sym_char(int c);
bool is_num_char(int c);
int  peek(FILE *in);
char read_char(FILE *in);
void skip_space(FILE* in);
Expr read_list(FILE *in);
Expr read_string(FILE* in);
Expr read_atom(FILE* in);

// Global symbol table could be added here

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
  else if ( c == '(' )
    x = read_list(in);
  else if ( c == '"')
    x = read_string(in);
  else if ( is_sym_char(c) )
    x = read_atom(in);
  else if ( c == ')' )
    runtime_error("dangling ')'");
  else
    runtime_error("unrecognized character %c", c);

  return x;
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
int  op_arity(OpCode op);
void emit_instr(Buffer* code, OpCode op, ...);
void compile_literal(Expr x, Alist* vals, Buffer* code);
void compile_global(Sym* s, Buffer* code);
void compile_funcall(List* form, Alist* vals, Buffer* code);
void compile_expr(Expr x, Alist* vals, Buffer* code);
Fun* toplevel_compile(List* form);

int op_arity(OpCode op) {
  int n;
  
  switch ( op ) {
    case OP_GET_VALUE:
    case OP_GET_GLOBAL:
    case OP_CALL:
      n = 1;
      break;

    default:
      n = 0;
      break;
  }

  return n;
}

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

void compile_literal(Expr x, Alist* vals, Buffer* code) {
  int n = alist_push(vals, x);

  emit_instr(code, OP_GET_VALUE, n);
}

void compile_global(Sym* s, Buffer* code) {
  int n = env_put(&Globals, s);

  emit_instr(code, OP_GET_GLOBAL, n);
}

void compile_funcall(List* form, Alist* vals, Buffer* code) {
  assert(form->count > 0);
  
  int argc = form->count-1;

  while ( form->count > 0 ) {
    Expr arg = form->head;
    compile_expr(arg, vals, code);
    form = form->tail;
  }

  emit_instr(code, OP_CALL, argc);
}

void compile_expr(Expr x, Alist* vals, Buffer* code) {
  ExpType t = exp_type(x);

  if ( t == EXP_SYM ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) ) // keywords (symbols whose names begin with ':') are treated as literals
      compile_literal(x, vals, code);

    else
      compile_global(s, code);
  } else if ( t == EXP_LIST ) {
    List* l = as_list(x);

    if ( l->count == 0 ) // empty list is treated as a literal
      compile_literal(x, vals, code);

    else
      compile_funcall(l, vals, code);
  } else
    compile_literal(x, vals, code);
}

Fun* toplevel_compile(List* form) {
  preserve(3, tag_obj(form), NUL, NUL);

  Alist* vals  = mk_alist();  add_to_preserved(tag_obj(vals), 1);
  Buffer* code = mk_buffer(); add_to_preserved(tag_obj(code), 2);

  compile_funcall(form, vals, code);
  emit_instr(code, OP_RETURN);

  Chunk* chunk = mk_chunk(vals, code); add_to_preserved(tag_obj(chunk), 1);
  Fun* out     = mk_user_fun(chunk);

  return out;
}


// exec
Expr exec_code(Fun* fun) {
  void* labels[] = {
    [OP_GET_VALUE]  = &&op_get_value,
    [OP_GET_GLOBAL] = &&op_get_global,
    [OP_ADD]        = &&op_add,
    [OP_SUB]        = &&op_sub,
    [OP_MUL]        = &&op_mul,
    [OP_DIV]        = &&op_div,
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
  
 op_add:
  require(argc == 2, "expected 2 arguments to +, got %d", argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s(x);
  ny     = as_num_s(y);
  nz     = nx + ny;
  z      = tag_num(nz);
  *tos() = z;           // combine push/pop

  goto fetch;

 op_sub:
  require(argc == 2, "expected 2 arguments to -, got %d", argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s(x);
  ny     = as_num_s(y);
  nz     = nx - ny;
  z      = tag_num(nz);
  *tos() = z;           // combine push/pop

  goto fetch;

 op_mul:
  require(argc == 2, "expected 2 arguments to *, got %d", argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s(x);
  ny     = as_num_s(y);
  nz     = nx * ny;
  z      = tag_num(nz);
  *tos() = z;           // combine push/pop

  goto fetch;

 op_div:
  require(argc == 2, "expected 2 arguments to /, got %d", argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s(x);
  ny     = as_num_s(y); require(ny != 0, "division by zero");
  nz     = nx / ny;
  z      = tag_num(nz);
  *tos() = z;           // combine push/pop

  goto fetch;  

 op_call:
  argc = next_op();
  x = *stack_ref(-argc-1);
  fun = as_fun_s(x);

  assert(fun->label != OP_NOOP); // user functions not yet supported

  op = fun->label;

  goto *labels[op];
  
 op_return:
  x = pop(); reset_vm();
  
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
