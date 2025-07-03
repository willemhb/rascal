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
  // syntax flags (indicates whether a particular form is restricted in the current context)
  CF_DEF      = 0x01, // allow def forms
  CF_PUT      = 0x02, // allow put forms

  // other compiler flags
  CF_TOPLEVEL = 0x80, // form is compiling at toplevel
};

// Function prototypes --------------------------------------------------------
bool is_delim_char(int c);
bool is_sym_char(int c);
bool is_num_char(int c);
void skip_space(Port* in);
Expr read_glyph(Port* in);
Expr read_quote(Port* in);
Expr read_list(Port *in);
Expr read_string(Port* in);
Expr read_atom(Port* in);

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

    if ( !isspace(c) ) {
      pungetc(p, c);
      break;
    }
  }
}

// read -----------------------------------------------------------------------
Expr read_exp(Port *in) {
  reset_token();
  skip_space(in);
  Expr x;
  int c = ppeekc(in);

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

Expr read_glyph(Port* in) {
  pgetc(in); // consume opening slash

  if ( peof(in) || isspace(ppeekc(in)) )
    eval_error("invalid syntax: empty character");

  Glyph g; int c;

  if ( !isalpha(ppeekc(in)) ) {
    g = ppeekc(in);
    c = ppeekc(in);

    require(isspace(c) || is_delim_char(c), "invalid character literal");
  }

  else {
    while (!peof(in) && !isspace(c=ppeekc(in)) && !is_delim_char(c) ) {
      add_to_token(c);
      ppeekc(in);
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

Expr read_quote(Port* in) {
  ppeekc(in); // consume opening '

  if ( peof(in) || isspace(ppeekc(in)) )
    eval_error("invalid syntax: quoted nothing");

  Sym* hd  = mk_sym("quote"); preserve(2, tag_obj(hd), NUL);
  Expr x   = read_exp(in);    add_to_preserved(1, x);
  List* qd = mk_list(2, preserved());

  return tag_obj(qd);
}

Expr read_list(Port* in) {
  List* out;
  ppeekc(in); // consume the '('
  skip_space(in);
  Expr* base = &Vm.vals[Vm.sp], x;
  int n = 0, c;

  while ( (c=ppeekc(in)) != ')' ) {
    if ( peof(in) )
      runtime_error("unterminated list");

    x = read_exp(in);
    vpush(x);
    n++;
    skip_space(in);
  }

  ppeekc(in); // consume ')'

  out = mk_list(n, base);

  if ( n > 0 )
    vpopn(n);

  return tag_obj(out);
}

Expr read_string(Port* in) {
  Str* out;

  int c;

  ppeekc(in); // consume opening '"'

  while ( (c=ppeekc(in)) != '"' ) {
    if ( peof(in) )
      runtime_error("unterminated string");

    add_to_token(c); // accumulate
    ppeekc(in);   // advance
  }

  ppeekc(in); // consume terminal '"'

  out = mk_str(Token);

  return tag_obj(out);
}

Expr read_atom(Port* in) {
  int c;
  Expr x;
  
  while ( !peof(in) && is_sym_char(c=ppeekc(in)) ) {
    add_to_token(c); // accumulate
    ppeekc(in);   // consume character
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

// compile --------------------------------------------------------------------
// compile helpers ------------------------------------------------------------
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
Fun* toplevel_compile(List* form);
Fun* compile_file(List* forms);

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

// exec
bool is_falsey(Expr x) {
  return x == NONE || x == NUL || x == FALSE;
}

Expr exec_code(Fun* fun) {
  void* labels[] = {
    [OP_NOOP]        = &&op_noop,

    // stack manipulation -----------------------------------------------------
    [OP_POP]         = &&op_pop,
    [OP_RPOP]        = &&op_rpop,

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

    // exception interface instructions ---------------------------------------
    [OP_CATCH]       = &&op_catch,
    [OP_THROW]       = &&op_throw,
    [OP_ECATCH]      = &&op_ecatch,

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

    // sequence operations ----------------------------------------------------
    [OP_CONS]        = &&op_cons,
    [OP_HEAD]        = &&op_head,
    [OP_TAIL]        = &&op_tail,
    [OP_NTH]         = &&op_nth,

    // system instructions ----------------------------------------------------
    [OP_HEAP_REPORT] = &&op_heap_report,
    [OP_DIS]         = &&op_dis,
  };

  int argc, argx, argy;
  OpCode op;
  Expr x, y, z;
  Num nx, ny, nz;
  List* lx, * ly;
  Status sig;

  install_fun(fun, 0);

 fetch:
  op = next_op();

  goto *labels[op];

  // miscellaneous instructions -----------------------------------------------
 op_noop:
  goto fetch;

  // stack manipulation instructions ------------------------------------------
 op_pop: // remove TOS
  vpop();

  goto fetch;

 op_rpop: // move TOS to TOS-1, then decrement sp
  vrpop();

  goto fetch;

  // value/variable instructions ----------------------------------------------
 op_get_value: // load a value from the constant store
  argx = next_op();
  x    = alist_get(Vm.fn->chunk->vals, argx);

  vpush(x);

  goto fetch;

 op_get_global:
  argx = next_op(); // previously resolved index in global environment
  x    = toplevel_env_ref(&Globals, argx);

  require(x != NONE, "undefined reference");
  vpush(x);

  goto fetch;

 op_set_global:
  argx = next_op();
  x    = vpop();

  toplevel_env_refset(&Globals, argx, x);

  goto fetch;

 op_get_local:
  argx = next_op();
  x    = Vm.vals[Vm.bp+argx];

  vpush(x);

  goto fetch;

 op_set_local:
  argx = next_op();
  x    = vpop();
  Vm.vals[Vm.bp+argx] = x;

  goto fetch;

 op_get_upval:
  argx = next_op();
  x    = upval_ref(Vm.fn, argx);

  vpush(x);

  goto fetch;

 op_set_upval:
  argx = next_op();
  x    = vpop();

  upval_set(Vm.fn, argx, x);

  goto fetch;

  // branching instructions ---------------------------------------------------
 op_jump: // unconditional jumping
  argx   = next_op();
  Vm.pc += argx;

  goto fetch;

 op_jump_f: // jump if TOS is falsey
  argx   = next_op();
  x      = vpop();

  if ( is_falsey(x) )
    Vm.pc += argx;

  goto fetch;

  // exception interface instructions -----------------------------------------
 op_catch:
  // handler should be TOS
  // save current execution context
  save_ctx();

  sig = safepoint();

  if ( sig ) {
    // argument to handler
    x = sig > USER_ERROR ? tag_obj(ErrorTypes[sig]) : tos();

    recover(NULL);
    discard_ctx();

    // set up call to handler
    argc  = 1;
    fun   = as_fun(tos());
    tos() = x;

    // jump to handler
    goto call_user_fn;
  }

  // unconditional jump that will skip over
  // the catch body if the handler is invoked
  Vm.pc += 2;

  // enter catch body
  goto fetch;

 op_throw:
  user_error("");

 op_ecatch:
  discard_ctx(); // discard saved context
  vrpop();       // discard handler and leave last expression of catch body at tos

  goto fetch;

  // closures and function calls ----------------------------------------------
 op_call:
  argc = next_op();
  x    = vpop();
  fun  = as_fun_s(Vm.fn->name->val->val, x);

  if ( is_user_fn(fun) )
    goto call_user_fn;

  op = fun->label;

  goto *labels[op];

 call_user_fn:
  require_argco("fn", user_fn_argc(fun), argc);
  save_frame();                                 // save caller state
  install_fun(fun, Vm.sp-argc);

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
      fun->upvs.vals[i] = get_upv(Vm.vals+Vm.bp+argy);
  }

  goto fetch;

  // emmitted before a frame with local upvalues returns
 op_capture:
  close_upvs(Vm.vals+Vm.bp);

  goto fetch;

 op_return:
  x = Vm.sp > 0 ? vpop() : NUL;

  if ( Vm.fp == 0 ) { // no calling frame, exit
    reset_vm();
    return x;
  }

  restore_frame();
  vpush(x);

  goto fetch;

  // builtin functions --------------------------------------------------------
  // at some hypothetical point in --------------------------------------------
  // the future these will be inlineable --------------------------------------
 op_add:
  require_argco("+", 2, argc);

  y      = vpop();
  x      = vpop();
  nx     = as_num_s("+", x);
  ny     = as_num_s("+", y);
  nz     = nx + ny;
  z      = tag_num(nz);

  vpush(z);
  
  goto fetch;

 op_sub:
  require_argco("-", 2, argc);

  y      = vpop();
  x      = vpop();
  nx     = as_num_s("-", x);
  ny     = as_num_s("-", y);
  nz     = nx - ny;
  z      = tag_num(nz);

  vpush(z);

  goto fetch;

 op_mul:
  require_argco("*", 2, argc);

  y      = vpop();
  x      = vpop();
  nx     = as_num_s("*", x);
  ny     = as_num_s("*", y);
  nz     = nx * ny;
  z      = tag_num(nz);

  vpush(z);

  goto fetch;

 op_div:
  require_argco("/", 2, argc);

  y      = vpop();
  x      = vpop();
  nx     = as_num_s("/", x);
  ny     = as_num_s("/", y); require(ny != 0, "division by zero");
  nz     = nx / ny;
  z      = tag_num(nz);

  vpush(z);

  goto fetch;

 op_egal:
  require_argco("=", 2, argc);

  y      = vpop();
  x      = vpop();
  z      = egal_exps(x, y) ? TRUE : FALSE;

  vpush(z);

  goto fetch;

 op_type:
  require_argco("type", 1, argc);

  x      = vpop();
  y      = tag_obj(exp_info(x)->repr);

  vpush(y);

  goto fetch;

 op_cons:
  require_argco("cons", 2, argc);

  lx = as_list_s("cons", Vm.vals[Vm.sp-1]);
  ly = cons(Vm.vals[Vm.sp-2], lx);
  z  = tag_obj(ly);

  vpopn(2);
  vpush(z);

  goto fetch;

 op_head:
  require_argco("head", 1, argc);

  x  = vpop();
  lx = as_list_s("head", x);

  require(lx->count > 0, "can't call head on empty list");

  y  = lx->head;

  vpush(y);

  goto fetch;

 op_tail:
  require_argco("tail", 1, argc);

  x  = vpop();
  lx = as_list_s("tail", x);

  require(lx->count > 0, "can't call tail on empty list");

  ly = lx->tail;

  vpush(tag_obj(ly));

  goto fetch;

 op_nth:
  require_argco("nth", 2, argc);

  x     = vpop();
  argx  = as_num_s("nth", x);
  x     = vpop();
  lx    = as_list_s("nth", x);

  require(argx < (int)lx->count, "index out of bounds");

  x = list_ref(lx, argx);

  vpush(x);

  goto fetch;

 op_heap_report:
  require_argco("*heap-report*", 0, argc);

  heap_report();

  vpush(NUL); // dummy return value

  goto fetch;

 op_dis:
  require_argco("*dis*", 1, argc);
  
  x   = vpop();
  fun = as_fun_s("*dis*", x);

  require(is_user_fn(fun), "can't disassemble builtin function");
  disassemble(fun);

  vpush(NUL);

  goto fetch;
}

// eval -----------------------------------------------------------------------
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

// print ----------------------------------------------------------------------
void print_exp(Port* out, Expr x) {
  ExpTypeInfo* info = exp_info(x);

  if ( info->print_fn )
    info->print_fn(out, x);

  else
    pprintf(out, "<%s>", info->name);
}

// load -----------------------------------------------------------------------
List* read_file(Port* in) {
  // return a list of all the expressions in a file
  List* out  = NULL;
  int   n    = 0; 
  Expr* base = &Vm.vals[Vm.sp];

  while ( !peof(in) ) {
    Expr x = read_exp(in);

    vpush(x);
    n++;
  }

  out = mk_list(n, base);

  vpopn(n);

  return out;
}

Expr load_file(char* fname) {
  // setup save point
  save_ctx();

  Port* in = NULL;
  Expr out = NUL;

  if ( safepoint() ) {
    recover(NULL);
  } else {
    in          = open_port(fname, "r");
    List* exprs = read_file(in);
    Fun*  code  = compile_file(exprs);
    out         = exec_code(code);
  }

  // clean up port (if necessary)
  if ( in )
    close_port(in);

  // discard save point
  discard_ctx();

  return out;
}

// repl -----------------------------------------------------------------------
void repl(void) {
  Expr x, v;

  // clean up the token buffer and ensure invalid input
  // is cleared from input stream
  void cleanup(void) {
    reset_token();
    pseek(&Ins, SEEK_SET, SEEK_END);
  }

  // create a catch point
  save_ctx();

  for (;;) {
    if ( safepoint() )
      recover(cleanup);

    else {
      fprintf(stdout, PROMPT" ");
      x = read_exp(&Ins);
      v = eval_exp(x);
      fprintf(stdout, "\n>>> ");
      print_exp(&Outs, v);
      fprintf(stdout, "\n\n");
    }
  }

  // discard catch point
  discard_ctx();
}
