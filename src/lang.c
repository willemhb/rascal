#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "runtime.h"
#include "opcode.h"
#include "util.h"
#include "lang.h"

// globals
Str* QuoteStr, *DefStr, * PutStr, * IfStr, * DoStr, * FnStr;

// Function prototypes
// read helpers
bool is_delim_char(int c);
bool is_sym_char(int c);
bool is_num_char(int c);
int  peek(FILE *in);
char read_char(FILE *in);
void skip_space(FILE* in);
Expr read_glyph(FILE* in);
Expr read_quote(FILE* in);
Expr read_list(FILE *in);
Expr read_string(FILE* in);
Expr read_atom(FILE* in);

// read helpers
bool is_delim_char(int c) {
  return strchr("(){}[]", c);
}

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
  else if ( c == '\\' )
    x = read_glyph(in);
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

Expr read_glyph(FILE* in) {
  read_char(in); // consume opening slash

  if ( feof(in) || isspace(peek(in)) )
    eval_error("invalid syntax: empty character");

  Glyph g; int c;

  if ( !isalpha(peek(in)) ) {
    g = read_char(in);
    c = peek(in);

    require(isspace(c) || is_delim_char(c), "invalid character literal");
  }

  else {
    while (!feof(in) && !isspace(c=peek(in)) && !is_delim_char(c) ) {
      add_to_token(c);
      read_char(in);
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

Expr read_quote(FILE* in) {
  read_char(in); // consume opening '

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
void emit_instr(Buf16* code, OpCode op, ...);
void fill_instr(Buf16* code, int offset, int val);

bool is_quote_form(List* form);
void compile_quote(List* form, Env* vars, Alist* vals, Buf16* code);
bool is_def_form(List* form);
void compile_def(List* form, Env* vars, Alist* vals, Buf16* code);
bool is_put_form(List* form);
void compile_put(List* form, Env* vars, Alist* vals, Buf16* code);
bool is_if_form(List* form);
void compile_if(List* form, Env* vars, Alist* vals, Buf16* code);
bool is_do_form(List* form);
void compile_do(List* form, Env* vars, Alist* vals, Buf16* code);
bool is_fn_form(List* form);
void compile_fn(List* form, Env* vars, Alist* vals, Buf16* code);
void compile_closure(Buf16* c_code, Env* vars, Alist* vals, Buf16* code);

void compile_sequence(List* exprs, Env* vars, Alist* vals, Buf16* code);
void compile_literal(Expr x, Env* vars, Alist* vals, Buf16* code);
void compile_reference(Sym* s, Env* ref, Alist* vals, Buf16* code);
void compile_funcall(List* form, Env* vars, Alist* vals, Buf16* code);
void compile_expr(Expr x, Env* vars, Alist* vals, Buf16* code);
Fun* toplevel_compile(List* form);

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

void compile_quote(List* form, Env* vars, Alist* vals, Buf16* code) {
  require_argco("quote", 1, form->count-1);

  Expr x = form->tail->head;

  compile_literal(x, vars, vals, code);
}

bool is_def_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == DefStr;
}

void compile_def(List* form, Env* vars, Alist* vals, Buf16* code) {
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

      if ( i >= vars->arity ) // account for call frame
        i += FRAME_SIZE;

      break;

    default:
      unreachable();
  }
  
  compile_expr(form->tail->tail->head, vars, vals, code);
  emit_instr(code, op, i);
}

bool is_put_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == PutStr;
}

void compile_put(List* form, Env* vars, Alist* vals, Buf16* code) {
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
  
  compile_expr(form->tail->tail->head, vars, vals, code);
  emit_instr(code, op, i);
}

bool is_if_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == IfStr;
}

void compile_if(List* form, Env* vars, Alist* vals, Buf16* code) {
    require_argco2("if", 2, 3, form->count-1);

    Expr test = form->tail->head;
    Expr then = form->tail->tail->head;
    Expr alt  = form->count == 3 ? NUL : form->tail->tail->tail->head;

    // compile different parts of the form, saving offsets to fill in later
    compile_expr(test, vars, vals, code);
    emit_instr(code, OP_JUMP_F, 0);
    int off1 = code->binary.count;
    compile_expr(then, vars, vals, code);
    emit_instr(code, OP_JUMP, 0);
    int off2 = code->binary.count;
    compile_expr(alt, vars, vals, code);
    int off3 = code->binary.count;

    fill_instr(code, off1-1, off2-off1);
    fill_instr(code, off2-1, off3-off2);
}

bool is_do_form(List* form) {
  Expr hd = form->head;

  return is_sym(hd) && as_sym(hd)->val == DoStr;
}

void compile_do(List* form, Env* vars, Alist* vals, Buf16* code) {
  require_vargco("do", 2, form->count-1);

  List* xprs = form->tail;

  compile_sequence(xprs, vars, vals, code);
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

void compile_fn(List* form, Env* vars, Alist* vals, Buf16* code) {
  require_vargco("fn", 1, form->count-1);

  List* argl  = as_list_s("fn", form->tail->head);
  List* body  = form->tail->tail;
  Env*  lvars = mk_env(vars);

  preserve(3, tag_obj(lvars), NUL, NUL);
  prepare_env(argl, lvars);

  Alist* lvals = mk_alist(); add_to_preserved(1, tag_obj(lvals));
  Buf16* lcode = mk_buf16(); add_to_preserved(2, tag_obj(lcode));

  // compile internal definitions first (otherwise their values will get lost in the stack)
  while ( body->count > 0 ) {
    Expr hd = body->head;

    if ( !is_list(hd) )
      break;

    List* fxpr = as_list(hd);

    if ( !is_def_form(fxpr) )
      break;

    compile_def(fxpr, lvars, lvals, lcode);
    body = body->tail;
  }

  // compile remaining expressions like body of a 'do' form
  compile_sequence(body, lvars, lvals, lcode);

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
  compile_literal(tag_obj(fun), vars, vals, code);

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

    // write arguments to closure ocpode at once
    buf16_write(code, buffer, upvc*2);
  }
}

void compile_sequence(List* xprs, Env* vars, Alist* vals, Buf16* code) {
  while ( xprs->count > 0 ) {
    Expr x = xprs->head;
    compile_expr(x, vars, vals, code);

    if ( xprs->count > 1 )
      emit_instr(code, OP_POP);

    xprs = xprs->tail;
  }
}

void compile_literal(Expr x, Env* vars, Alist* vals, Buf16* code) {
  (void)vars;
  int n = alist_push(vals, x);

  emit_instr(code, OP_GET_VALUE, n-1);
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

      if ( i >= vars->arity )
        i += FRAME_SIZE;

      break;

    case REF_CAPTURED_UPVAL:
      op = OP_GET_UPVAL;
      break;

    default:
      unreachable();
  }

  emit_instr(code, op, i);
}

void compile_funcall(List* form, Env* vars, Alist* vals, Buf16* code) {
  assert(form->count > 0);

  if ( is_quote_form(form) )
    compile_quote(form, vars, vals, code);

  else if ( is_def_form(form) ) {
    require(!is_local_env(vars), "syntax error: local def in fn body");
    compile_def(form, vars, vals, code);
  }

  else if ( is_put_form(form) )
    compile_put(form, vars, vals, code);

  else if ( is_if_form(form) )
    compile_if(form, vars, vals, code);

  else if ( is_do_form(form) )
    compile_do(form, vars, vals, code);

  else if ( is_fn_form(form) )
    compile_fn(form, vars, vals, code);

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

void compile_expr(Expr x, Env* vars, Alist* vals, Buf16* code) {
  ExpType t = exp_type(x);
  
  if ( t == EXP_SYM ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) ) // keywords (symbols whose names begin with ':') are treated as literals
      compile_literal(x, vars, vals, code);

    else
      compile_reference(s, vars, vals, code);

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
  Buf16* code = mk_buf16(); add_to_preserved(2, tag_obj(code));

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
    [OP_NOOP]        = &&op_noop,

    // stack manipulation
    [OP_POP]         = &&op_pop,

    // environment instructions
    [OP_GET_VALUE]   = &&op_get_value,
    [OP_GET_GLOBAL]  = &&op_get_global,
    [OP_SET_GLOBAL]  = &&op_set_global,
    [OP_GET_LOCAL]   = &&op_get_local,
    [OP_SET_LOCAL]   = &&op_set_local,
    [OP_GET_UPVAL]   = &&op_get_upval,
    [OP_SET_UPVAL]   = &&op_set_upval,

    // jump instructions
    [OP_JUMP]        = &&op_jump,
    [OP_JUMP_F]      = &&op_jump_f,

    // function call instructions
    [OP_CLOSURE]     = &&op_closure,
    [OP_CAPTURE]     = &&op_capture,
    [OP_CALL]        = &&op_call,
    [OP_RETURN]      = &&op_return,


    // arithmetic instructions
    [OP_ADD]         = &&op_add,
    [OP_SUB]         = &&op_sub,
    [OP_MUL]         = &&op_mul,
    [OP_DIV]         = &&op_div,

    // miscellaneous builtins
    [OP_EGAL]        = &&op_egal,
    [OP_TYPE]        = &&op_type,

    // sequence operations
    [OP_CONS]        = &&op_cons,
    [OP_HEAD]        = &&op_head,
    [OP_TAIL]        = &&op_tail,
    [OP_NTH]         = &&op_nth,

    // system instructions
    [OP_HEAP_REPORT] = &&op_heap_report,
  };

  int argc, argx, argy;
  OpCode op;
  Expr x, y, z;
  Num nx, ny, nz;
  List* lx, * ly;

  install_fun(fun, 0, 0);

 fetch:
  op = next_op();

  goto *labels[op];

 op_noop:
  goto fetch;
  
 op_pop: // remove TOS
  pop();

  goto fetch;

 op_get_value: // load a value from the constant store
  argx = next_op();
  x    = alist_get(Vm.fn->chunk->vals, argx);

  push(x);

  goto fetch;

 op_get_global:
  argx = next_op(); // previously resolved index in global environment
  x    = toplevel_env_ref(&Globals, argx);

  require(x != NONE, "undefined reference");
  push(x);

  goto fetch;

 op_set_global:
  argx = next_op();
  x    = pop();

  toplevel_env_refset(&Globals, argx, x);

  goto fetch;

 op_get_local:
  argx = next_op();
  x    = Vm.stack[Vm.bp+argx];

  push(x);

  goto fetch;

 op_set_local:
  argx = next_op();
  x    = pop();
  Vm.stack[Vm.bp+argx] = x;

  goto fetch;

 op_get_upval:
  argx = next_op();
  x    = upval_ref(Vm.fn, argx);

  push(x);

  goto fetch;

 op_set_upval:
  argx = next_op();
  x    = pop();

  upval_set(Vm.fn, argx, x);

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
  x    = *stack_ref(-argc-1);
  fun  = as_fun_s(Vm.fn->name->val->val, x);

  if ( is_user_fn(fun) )
    goto call_user_fn;

  op = fun->label;

  goto *labels[op];

 call_user_fn:
  require_argco("fn", user_fn_argc(fun), argc);
  save_frame();                                 // save caller state
  install_fun(fun, Vm.sp-argc-FRAME_SIZE, Vm.sp);

  goto fetch;

 op_closure:
  fun   = as_fun(tos());
  fun   = mk_closure(fun);
  tos() = tag_obj(fun);     // make sure new closure is visible to GC
  argc  = next_op();

  for ( int i=0; i < argc; i++ ) {
    argx = next_op();
    argy = next_op();

    if ( argx == 0 ) // nonlocal
      fun->upvs.vals[i] = Vm.fn->upvs.vals[argy];

    else
      fun->upvs.vals[i] = get_upv(Vm.stack+Vm.bp+argy);
  }

  goto fetch;

  // emmitted before a frame with local upvalues returns
 op_capture:
  close_upvs(Vm.stack+Vm.bp);

  goto fetch;

 op_return:
  x    = Vm.sp > Vm.fp ? pop() : NUL;

  if ( Vm.fp == 0 ) { // no calling frame, exit
    reset_vm();
    return x;    
  }

  argx  = Vm.bp;       // adjust stack to here after restore
  
  restore_frame();

  Vm.sp = argx;
  tos() = x;

  goto fetch;

  // builtin functions --------------------------------------------------------
  // at some hypothetical point in --------------------------------------------
  // the future these will be inlineable --------------------------------------
 op_add:
  require_argco("+", 2, argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s("+", x);
  ny     = as_num_s("+", y);
  nz     = nx + ny;
  z      = tag_num(nz);
  tos()  = z;           // combine push/pop

  goto fetch;

 op_sub:
  require_argco("-", 2, argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s("-", x);
  ny     = as_num_s("-", y);
  nz     = nx - ny;
  z      = tag_num(nz);
  tos()  = z;           // combine push/pop

  goto fetch;

 op_mul:
  require_argco("*", 2, argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s("*", x);
  ny     = as_num_s("*", y);
  nz     = nx * ny;
  z      = tag_num(nz);
  tos()  = z;           // combine push/pop

  goto fetch;

 op_div:
  require_argco("/", 2, argc);

  y      = pop();
  x      = pop();
  nx     = as_num_s("/", x);
  ny     = as_num_s("/", y); require(ny != 0, "division by zero");
  nz     = nx / ny;
  z      = tag_num(nz);
  tos()  = z;           // combine push/pop

  goto fetch;

 op_egal:
  require_argco("=", 2, argc);

  y      = pop();
  x      = pop();
  z      = egal_exps(x, y) ? TRUE : FALSE;
  tos()  = z;

  goto fetch;

 op_type:
  require_argco("type", 1, argc);

  x      = pop();
  y      = tag_obj(exp_info(x)->repr);
  tos()  = y;

  goto fetch;

 op_cons:
  require_argco("cons", 2, argc);

  lx = as_list_s("cons", Vm.stack[Vm.sp-1]);
  ly = cons(Vm.stack[Vm.sp-2], lx);
  z  = tag_obj(ly);

  popn(2);

  tos() = z;

  goto fetch;

 op_head:
  require_argco("head", 1, argc);

  x  = pop();
  lx = as_list_s("head", x);

  require(lx->count > 0, "can't call head on empty list");

  y  = lx->head;

  push(y);

  goto fetch;

 op_tail:
  require_argco("tail", 1, argc);

  x  = pop();
  lx = as_list_s("tail", x);

  require(lx->count > 0, "can't call tail on empty list");

  ly = lx->tail;

  push(tag_obj(ly));

  goto fetch;

 op_nth:
  require_argco("nth", 2, argc);

  x     = pop();
  argx  = as_num_s("nth", x);
  x     = pop();
  lx    = as_list_s("nth", x);

  require(argx < (int)lx->count, "index out of bounds");

  x     = list_ref(lx, argx);
  tos() = x;

  goto fetch;

 op_heap_report:
  require_argco("*heap-report*", 0, argc);

  heap_report();

  tos() = NUL; // dummy retunr value

  goto fetch;
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
      v = toplevel_env_get(&Globals, s);
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
