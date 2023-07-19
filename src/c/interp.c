#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "interp.h"
#include "object.h"
#include "lang.h"

#include "opcodes.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// special forms --------------------------------------------------------------
value_t Quote, Do, If, Lmb, Def, Put, Ccc;

// other syntactic markers ----------------------------------------------------
value_t Ampersand;

// special constants ----------------------------------------------------------
value_t True, False;

// internal API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static bool is_literal( value_t x ) {
  if ( is_symbol(x) )
    return hasfl(x, LITERAL);

  else if ( is_list(x) )
    return as_list(x)->arity > 0;

  else
    return true;
}

static value_t c_to_rl_bool( bool b ) {
  return b ? True : False;
}

static bool rl_to_c_bool( value_t x ) {
  return x != NIL && x != False;
}

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x ) {
  value_t v;

  if ( is_literal(x) )
    v = x;

  else if ( is_symbol(x) ) {
    v = table_get(&Vm.globals, x);
    forbid("eval", v == NOTFOUND, x, "unbound symbol");
  }

  else {
    chunk_t* chunk = compile(x);
    v = exec(chunk);
  }

  return v;
}

#define prompt "rascal>"

void repl( void ) {
  while ( true ) {
    if ( setjmp(Vm.context) ) {
      reset_interp();
      reset_reader();
    } else {
      printf(prompt" ");
      value_t x = read(stdin);
      printf("\n\n");
      value_t v = eval(x);
      print(stdout, v);
      printf("\n\n");
    }
  }
}

//  compiler
static usize emit_instr( chunk_t* target, opcode_t op, ... );
static void fill_jump( chunk_t* target, usize offset );
static usize add_value( chunk_t* target, value_t val );
static alist_t* next_environment( alist_t* env );
static void resolve_variable( chunk_t* target, value_t name, bool* toplevel, usize* i, usize* j );
static void extend_environment( chunk_t* target, value_t name, bool* toplevel, usize* i );
static alist_t* prepare_environment( chunk_t* target, list_t* formals, bool* variadic );
static usize compile_value( chunk_t* target, value_t val );
static usize compile_variable( chunk_t* target, value_t name );
static usize compile_combination( chunk_t* target, value_t sexpr );
static usize compile_assignment( chunk_t* target, value_t name, value_t val );
static usize compile_definition( chunk_t* target, value_t name, value_t val );
static usize compile_closure( chunk_t* target, alist_t* envt, list_t* exprs, bool variadic );
static usize compile_expr( chunk_t* target, value_t src );
static usize compile_exprs( chunk_t* target, list_t* exprs );
static usize compile_args( chunk_t* target, list_t* args );
static usize compile_quote( chunk_t* target, list_t* form );
static usize compile_do( chunk_t* target, list_t* form );
static usize compile_if( chunk_t* target, list_t* form );
static usize compile_def( chunk_t* target, list_t* form );
static usize compile_put( chunk_t* target, list_t* form );
static usize compile_lmb( chunk_t* target, list_t* form );
static usize compile_ccc( chunk_t* target, list_t* form );

chunk_t* compile( value_t src ) {
  chunk_t* target = mk_chunk( NULL );
  compile_expr(target, src);
  emit_instr(target, OP_RETURN);
  return target;
}

static usize emit_instr( chunk_t* target, opcode_t op, ... ) {
  va_list va;
  va_start(va, op);

  ushort buf[3] = { op, 0, 0 };
  usize n = opcode_argc(op);

  if ( n > 0 )
    buf[1] = va_arg(va, int);

  if ( n > 1 )
    buf[2] = va_arg(va, int);

  va_end(va);
  buffer_write(target->code, n+1, buf);
  return target->code->cnt;
}

static void fill_jump( chunk_t* target, usize offset ) {
  ((ushort*)target->code->data)[offset-1] = target->code->cnt - offset;
}

static usize add_value( chunk_t* target, value_t val ) {
  return alist_push(target->vals, val);
}

static alist_t* next_environment( alist_t* env ) {
  if ( env == NULL )
    return NULL;

  if ( env->data[0] == NIL )
    return NULL;

  return as_alist(env->data[0]);
}

static void resolve_variable( chunk_t* target, value_t name, bool* toplevel, usize* i, usize* j ) {
  *toplevel = false;
  *i = *j = 0;

  alist_t* env = target->vars;

  while ( env && *j == 0 ) {
    for ( usize k=1; k<env->cnt && *j == 0; k++ )
      if ( env->data[k] == name )
        *j = k;

    if ( *j == 0 ) { // not found in this frame
      env = next_environment(env);
      (*i)++;
    }
  }

  if ( *j == 0 ) {
    *toplevel = true;
    *i = add_value(target, name);
  }
}

static void extend_environment( chunk_t* target, value_t name, bool* toplevel, usize* i ) {
  if ( target->vars == NULL ) {
    *toplevel = true;
    *i = add_value(target, name);
  } else {
    *toplevel = false;
    *i = 0;

    alist_t* env = target->vars;

    for ( usize k=1; k<env->cnt && *i == 0; k++ )
      if ( env->data[k] == name )
        *i = k;

    if ( *i == 0 )
      *i = alist_push(env, name);
  }
}

static alist_t* prepare_environment( chunk_t* target, list_t* formals, bool* variadic ) {
  *variadic = false;
  list_t* curr = formals;
  value_t buffer[formals->arity+1];
  buffer[0] = target->vars == NULL ? NIL : object(target->vars);
  usize n = 1;

  while ( curr->arity ) {
    value_t name = curr->head;
    require("compile", is_symbol(name), object(formals), "Formal argument is not a symbol");

    if ( name == Ampersand ) {
      require("compile", formals->arity == 2, object(formals), "Illicit use of '&' syntax");
      *variadic = true;
    } else {
      for ( usize i=1; i<n; i++ )
        require("compile", name != buffer[i], object(formals), "Symbol appears multiple times in formal arguments list");

      buffer[n++] = name;
    }

    curr = curr->tail;
  }

  return mk_alist(n, buffer);
}

static usize compile_value( chunk_t* target, value_t val ) {
  usize out;

  if ( val == NIL )
    out = emit_instr(target, OP_LOADN);

  else {
    usize location = add_value(target, val);
    out = emit_instr(target, OP_LOADV, location);
  }

  return out;
}

static usize compile_variable( chunk_t* target, value_t name ) {
  usize out;
  usize i, j;
  bool toplevel;
  resolve_variable(target, name, &toplevel, &i, &j);

  if ( toplevel )
    out = emit_instr(target, OP_LOADGL, i);

  else
    out = emit_instr(target, OP_LOADCL, i, j);

  return out;
}

static usize compile_combination( chunk_t* target, value_t sexpr ) {
  list_t* form = as_list(sexpr);

  if ( form->head == Quote )
    return compile_quote(target, form);

  if ( form->head == Do )
    return compile_do(target, form);

  if ( form->head == If )
    return compile_if(target, form);

  if ( form->head == Lmb )
    return compile_lmb(target, form);

  if ( form->head == Def )
    return compile_def(target, form);

  if ( form->head == Put )
    return compile_put(target, form);

  if ( form->head == Ccc )
    return compile_ccc(target, form);

  compile_expr(target, form->head); // compile caller
  compile_args(target, form->tail); // compile arguments
  return emit_instr(target, OP_CALL, form->tail->arity);
}

static usize compile_assignment( chunk_t* target, value_t name, value_t val ) {
  bool toplevel;
  usize i, j;
  usize out;

  compile_expr(target, val);
  resolve_variable(target, name, &toplevel, &i, &j);

  if ( toplevel )
    out = emit_instr(target, OP_PUTGL, i);

  else
    out = emit_instr(target, OP_PUTCL, i, j);

  return out;
}

static usize compile_definition( chunk_t* target, value_t name, value_t val ) {
  bool toplevel;
  usize i;
  usize out;

  extend_environment(target, name, &toplevel, &i);

  if ( toplevel ) {
    emit_instr(target, OP_DEFGL, i);
    compile_expr(target, val);
    out = emit_instr(target, OP_PUTGL, i);
  }

  else {
    compile_expr(target, val);
    out = emit_instr(target, OP_PUTCL, 0, i);
  }

  return out;
}

static usize compile_closure( chunk_t* target, alist_t* envt, list_t* exprs, bool variadic ) {
  chunk_t* closure = mk_chunk(envt);
  emit_instr(closure, variadic ? OP_VARGC : OP_ARGC, envt->cnt-1);
  compile_exprs(closure, exprs);
  emit_instr(closure, OP_RETURN);
  compile_value(target, object(closure));
  return emit_instr(target, OP_CLOSURE);
}

static usize compile_expr( chunk_t* target, value_t expr ) {
  usize out;

  if ( is_literal(expr) )
    out = compile_value(target, expr);

  else if ( is_symbol(expr) )
    out = compile_variable(target, expr);

  else
    out = compile_combination(target, expr);

  return out;
}

static usize compile_exprs( chunk_t* target, list_t* exprs ) {
  while ( exprs->arity > 1 ) {
    compile_expr(target, exprs->head);
    emit_instr(target, OP_POP);
    exprs = exprs->tail;
  }

  return compile_expr(target, exprs->head);
}

static usize compile_args( chunk_t* target, list_t* args ) {
  usize out = target->code->cnt;

  while ( args->arity ) {
    out = compile_expr(target, args->head);
    args = args->tail;
  }

  return out;
}

static usize compile_quote( chunk_t* target, list_t* form ) {
  require("compile", form->arity == 2, object(form), "Incorrect arity for quote");
  value_t quoted = list_nth(form, 2);
  return compile_value(target, quoted);
}

static usize compile_do( chunk_t* target, list_t* form ) {
  require("compile", form->arity >= 2, object(form), "Incorrect arity for do");
  return compile_exprs(target, form->tail);
}

static usize compile_if( chunk_t* target, list_t* form ) {
  require("compile", form->arity == 3 || form->arity == 4, object(form), "Incorrect arity for if");
  value_t test = form->tail->head;
  value_t consequent = form->tail->tail->head;
  value_t alternative = form->arity == 3 ? NIL : form->tail->tail->tail->head;
  usize offset;

  compile_expr(target, test);
  offset = emit_instr(target, OP_JUMPN, 0);
  compile_expr(target, consequent);
  fill_jump(target, offset);
  offset = emit_instr(target, OP_JUMP, 0);
  compile_expr(target, alternative);
  fill_jump(target, offset);

  return target->code->cnt;
}

static usize compile_def( chunk_t* target, list_t* form ) {
  require("compile", form->arity == 3, object(form), "Incorrect arity for def");
  value_t name = form->tail->head;
  require("compile", is_symbol(name), object(form), "Attempt to def non-symbol");
  value_t bind = form->tail->tail->head;
  return compile_definition(target, name, bind);
}

static usize compile_put( chunk_t* target, list_t* form ) {
  require("compile", form->arity == 3, object(form), "Incorrect arity for put");
  value_t name = form->tail->head;
  require("compile", is_symbol(name), object(form), "Attempt to put value of non-symbol");
  value_t bind = form->tail->tail->head;
  return compile_assignment(target, name, bind);
}

static usize compile_lmb( chunk_t* target, list_t* form ) {
  require("compile", form->arity >= 3, object(form), "Incorrect arity for lmb");
  value_t argl = form->tail->head;
  require("compile", is_list(argl), object(form), "Formal arguments not a list");
  bool variadic;
  alist_t* vars = prepare_environment(target, as_list(argl), &variadic);
  list_t* exprs = form->tail->tail;
  return compile_closure(target, vars, exprs, variadic);
}

static usize compile_ccc( chunk_t* target, list_t* form ) {
  require("compile", form->arity >= 3, object(form), "Incorrect arity for ccc");
  value_t ccs = form->tail->head;
  require("compile", is_symbol(ccs), object(form), "Continuation name is not a symbol");
  value_t buffer[2] = { target->vars == NULL ? NIL : object(target->vars), ccs };
  alist_t* envt = mk_alist(2, buffer);
  list_t* exprs = form->tail->tail;
  compile_closure(target, envt, exprs, false);
  emit_instr(target, OP_CONTROL);
  return emit_instr(target, OP_CALL, 1);
}

// exec helpers
static bool is_captured( frame_t* f ) {
  return !!(f->fl & CAPTURED);
}

static void capture_frame( frame_t* f ) {
  if ( !is_captured(f) ) {
    alist_t* env = mk_alist(f->fn->vars->cnt, &Values[f->bp]);
    Values[f->bp] = object(env);
    f->fl |= CAPTURED;
  }
}

static value_t do_exec( chunk_t* code, opcode_t entry ) {
  static void* labels[] = {
    [OP_NOOP] = &&op_noop,
    [OP_START] = &&op_start,
    [OP_POP] = &&op_pop,
    [OP_CLOSURE] = &&op_closure, [OP_CONTROL] = &&op_control,
    [OP_RETURN] = &&op_return,
    [OP_LOADN] = &&op_loadn,

    [OP_ARGC] = &&op_argc, [OP_VARGC] = &&op_vargc,
    [OP_CALL] = &&op_call,
    [OP_LOADV] = &&op_loadv,
    [OP_LOADGL] = &&op_loadgl, [OP_PUTGL] = &&op_putgl, [OP_DEFGL] = &&op_defgl,
    [OP_JUMP] = &&op_jump, [OP_JUMPN] = &&op_jumpn,

    [OP_LOADCL] = &&op_loadcl, [OP_PUTCL] = &&op_putcl
  };

  opcode_t op = entry;
  int argx, argy;
  value_t x, v, *b;
  list_t* va;

  goto *labels[op];

 fetch: // next instruction
  op = *(IP++);

  if ( op > OP_RETURN )
    argx = *(IP++);

  if ( op > OP_JUMPN )
    argy = *(IP++);

  goto *labels[op];

 op_start:
  push_frame();
  push(NIL);
  FN  = code;
  IP  = code->code->data;
  BP  = 0;
  FL  = 0;

  goto fetch;

 op_noop:
  goto fetch;

 op_pop:
  pop();
  goto fetch;

 op_closure:
  v = pop();
  capture_frame(&FRAME);
  v = object(mk_closure(as_chunk(v), as_alist(Values[BP])));
  push(v);
  goto fetch;

 op_control:
  v = object(mk_control(&FRAME, SP, FP, Frames, Values));
  push(v);
  goto fetch;

 op_return:
  v = pop();
  pop_frame();
  if ( FN == NULL )
    return v;
  push(v);
  goto fetch;

 op_loadn:
  push(NIL);
  goto fetch;

 op_argc:
  argy = SP - BP - 1;
  require("exec", argy == argx, object(FN), "Incorrect arity");
  goto pad_locals;

 op_vargc:
  argy = SP - BP - 1;
  require("exec", argy >= argx, object(FN), "Incorrect arity");
  va = mk_list(argy - argx, &Values[BP+1+argx]);
  Values[BP+1+argx] = object(va);
  SP = BP+argx+2;
  goto pad_locals;

 pad_locals: // adjust stack to hold all local bindings, including those introduced in function body
  for ( int i=argx; i<(int)FN->vars->cnt; i++ )
    push(NIL);

  goto fetch;

 op_call:
  x = Values[SP-argx-1];
  if ( is_native(x) )
    goto do_call_native;
  else if ( is_closure(x) )
    goto do_call_closure;
  else if ( is_control(x) )
    goto do_call_control;
  else
    error("exec", x, "not a function");

 do_call_native:
  v = as_native(x)(argx, &Values[SP-argx]);
  popn(argx+1);
  push(v);
  goto fetch;

 do_call_closure:
  push_frame();

  FN   = as_closure(x)->code;
  IP   = FN->code->data;
  BP   = SP - argx - 1;
  FL   = 0;
  ENV  = object(as_closure(x)->envt);

  goto fetch;

 do_call_control: {
    control_t* c = as_control(x);

    // save argument to continuation
    v = pop();

    // restore state where continuation was captured
    Vm.frame = c->frame;
    Vm.fp = c->fp;
    Vm.sp = c->sp;
    memcpy(Frames, c->frames, FP * sizeof(frame_t));
    memcpy(Values, c->values, SP * sizeof(value_t));

    // push continuation argument
    push(v);
  }

  goto fetch;
  
 op_loadv:
  push(VALS[argx]);
  goto fetch;
  
 op_loadgl:
  x = VALS[argx];
  v = table_get(&Vm.globals, x);
  require("exec", v != NOTFOUND, x, "unbound symbol");
  goto fetch;

 op_putgl:{
    x = VALS[argx];
    b = table_find(&Vm.globals, x);
    require("exec", b[0] != NOTFOUND, x, "unbound symbol");
    b[1] = TOS;
  }
  goto fetch;

 op_defgl:
  x = VALS[argx];
  table_add(&Vm.globals, x, NIL);
  goto fetch;

 op_jump:
  IP += argx;
  goto fetch;

 op_jumpn:
  x = pop();

  if ( x == NIL )
    IP += argx;

  goto fetch;

 op_loadcl:
  b = is_captured(&FRAME) ? as_alist(ENV)->data : &Values[BP];
    
  while ( argx-- )
    b = as_alist(ENV)->data;
  
  push(b[argy]);

  goto fetch;

 op_putcl:
  b = is_captured(&FRAME) ? as_alist(ENV)->data : &Values[BP];

  while ( argx-- )
    b = as_alist(ENV)->data;

  b[argy] = TOS;

  goto fetch;
}

value_t exec( chunk_t* code ) {
  return do_exec(code, OP_START);
}

// native functions
value_t native_idp( usize n, value_t* args ) {
  argco("id?", false, 2, n);
  return args[0] == args[1] ? True : NIL;
}

value_t native_eqp( usize n, value_t* args ) {
  argco("eq?", false, 2, n);
  return equal(args[0], args[1]) ? True : NIL;
}

value_t native_compare( usize n, value_t* args ) {
  argco("compare", false, 2, n);
  return number(compare(args[0], args[1]));
}

value_t native_hash( usize n, value_t* args ) {
  argco("hash", false, 1, n);
  return number(hash(args[0]));
}

value_t native_add( usize n, value_t* args ) {
  argco("+", true, 1, n);
  value_t out;
  
  if ( n == 1 ) {
    argtype("+", args[0], NUMBER);
    out = args[0];
  } else {
    number_t accum = to_number("+", args[0]);

    for ( usize i=1; i<n; i++ )
      accum += to_number("+", args[i]);

    out = number(accum);
  }

  return out;
}

value_t native_sub( usize n, value_t* args ) {
  argco("-", true, 1, n);
  value_t out;

  if ( n == 1 ) {
    argtype("-", args[0], NUMBER);
    out = number(-as_number(args[0]));
  } else {
    number_t accum = to_number("-", args[0]);

    for ( usize i=1; i<n; i++ )
      accum -= to_number("-", args[i]);

    out = number(accum);
  }

  return out;
}

value_t native_mul( usize n, value_t* args ) {
  argco("*", true, 2, n);

  number_t accum = to_number("*", args[0]);

  for ( usize i=1; i<n && accum != 0; i++ )
    accum *= to_number("*", args[i]);

  return number(accum);
}

value_t native_div( usize n, value_t* args ) {
  argco("/", true, 1, n);

  value_t out;

  if ( n == 1 ) {
    number_t denom = to_number("/", args[0]);
    require("/", denom != 0, args[0], "division by zero");
    out = number(1/denom);
  } else {
    number_t accum = to_number("/", args[0]);
    
    for ( usize i=1; i<n && accum != 0; i++ ) {
      number_t divisor = to_number("/", args[i]);
      require("/", divisor != 0, args[i], "division by zero");
      accum /= divisor;
    }
    out = number(accum);
  }

  return out;
}

value_t native_mod( usize n, value_t* args ) {
  argco("mod", false, 2, n);
  long numer = to_integer("mod", args[0]);
  long denom = to_integer("mod", args[1]);
  require("mod", denom != 0, args[1], "division by zero");
  return number(numer % denom);
}

value_t native_eql( usize n, value_t* args ) {
  argco("=", true, 2, n);
  argtype("=", args[0], NUMBER);

  value_t out = True;

  for ( usize i=1; out != False && i<n; i++ ) {
    argtype("=", args[i], NUMBER);

    if ( args[i] != args[i-1] )
      out = False;
  }

  return out;
}

value_t native_ltp( usize n, value_t* args ) {
  argco("<", true, 2, n);
  argtype("<", args[0], NUMBER);
  value_t out = True;

  for ( usize i=1; out != False && i<n; i++ ) {
    argtype("<", args[i], NUMBER);

    if ( as_number(args[i-1]) >= as_number(args[i]) )
      out = False;
  }

  return out;
}

#define NATIVE_TYPEP( type )                            \
  value_t native_##type##p( usize n, value_t* args )    \
  {                                                     \
    argco(#type"?", false, 1, n);                       \
    return c_to_rl_bool(is_##type(args[0]));            \
  }

NATIVE_TYPEP(number);
NATIVE_TYPEP(glyph);
NATIVE_TYPEP(unit);
NATIVE_TYPEP(port);
NATIVE_TYPEP(native);
NATIVE_TYPEP(pointer);
NATIVE_TYPEP(symbol);
NATIVE_TYPEP(list);
NATIVE_TYPEP(alist);
NATIVE_TYPEP(table);
NATIVE_TYPEP(buffer);
NATIVE_TYPEP(chunk);
NATIVE_TYPEP(closure);
NATIVE_TYPEP(control);

#undef NATIVE_TYPEP

// toplevel initialization
static void define_native( const char* fname, native_t fun ) {
  toplevel_define(fname, native(fun));
}

void toplevel_init_interp( void ) {
  // create special forms & constants +++++++++++++++++++++++++++++++++++++++++
  Quote     = object(symbol("quote"));
  Do        = object(symbol("do"));
  If        = object(symbol("if"));
  Lmb       = object(symbol("lmb"));
  Def       = object(symbol("def"));
  Put       = object(symbol("put"));
  Ccc       = object(symbol("ccc"));
  Ampersand = object(symbol("&"));
  True      = object(keyword("true"));
  False     = object(keyword("false"));
  
  // create native functions ++++++++++++++++++++++++++++++++++++++++++++++++++
  // comparison ---------------------------------------------------------------
  define_native("id?", native_idp);
  define_native("eq?", native_eqp);
  define_native("compare", native_compare);
  define_native("hash", native_hash);

  // arithmetic ---------------------------------------------------------------
  define_native("+", native_add);
  define_native("-", native_sub);
  define_native("*", native_mul);
  define_native("/", native_div);
  define_native("mod", native_mod);
  define_native("=", native_eql);
  define_native("<", native_ltp);

  // type predicates ----------------------------------------------------------
  define_native("number?", native_numberp);
  define_native("glyph?", native_glyphp);
    
  // constructors -------------------------------------------------------------

  // accessors/mutators -------------------------------------------------------

  // interpreter --------------------------------------------------------------
}
