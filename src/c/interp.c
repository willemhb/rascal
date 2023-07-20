#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "interp.h"
#include "object.h"
#include "lang.h"

#include "opcodes.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// special forms --------------------------------------------------------------
value_t Quote, Do, If, Lmb, Mac, Def, Put, Ccc;

// other syntactic markers ----------------------------------------------------
value_t Ampersand;

// special macro arguments ----------------------------------------------------
value_t Form, Env;

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

static bool is_function( value_t x ) {
  return is_native(x) || is_closure(x);
}

static value_t lookup( value_t name ) {
  value_t val = table_get(&Vm.globals, name);

  if ( val == NOTFOUND )
    val = UNDEFINED;

  return val;
}

static bool is_macro_call( list_t* form ) {
  value_t head = form->head;
  bool out = false;

  if ( is_symbol(head) ) {
    value_t val = lookup( head );

    if ( is_chunk(val) )
      out = hasfl(val, MACRO);
  }

  return out;
}

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x ) {
  value_t v;

  if ( is_literal(x) )
    v = x;

  else if ( is_symbol(x) ) {
    v = lookup(x);
    forbid("eval", v == UNDEFINED, x, "unbound symbol");
  } else {
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

//  compiler ------------------------------------------------------------------
static list_t* mk_environment( list_t* parent );
static usize emit_instr( chunk_t* target, opcode_t op, ... );
static void fill_jump( chunk_t* target, usize offset );
static usize add_value( chunk_t* target, value_t val );
static opcode_t resolve( value_t name, chunk_t* target, usize* i, usize* j );
static opcode_t extend( value_t name, chunk_t* target, usize* i );
static value_t update_environment( value_t name, list_t* envt );
static list_t* prepare_environment( list_t* parent, list_t* formals, bool* variadic );
static usize compile_value( chunk_t* target, value_t val );
static usize compile_variable( chunk_t* target, value_t name );
static usize compile_combination( chunk_t* target, value_t sexpr );
static usize compile_macro_call( chunk_t* target, value_t sexpr );
static usize compile_assignment( chunk_t* target, value_t name, value_t val );
static usize compile_definition( chunk_t* target, value_t name, value_t val );
static usize compile_closure( chunk_t* target, list_t* formals, list_t* exprs, bool variadic );
static usize compile_expr( chunk_t* target, value_t src );
static usize compile_exprs( chunk_t* target, list_t* exprs );
static usize compile_args( chunk_t* target, list_t* args );
static usize compile_quote( chunk_t* target, list_t* form );
static usize compile_do( chunk_t* target, list_t* form );
static usize compile_if( chunk_t* target, list_t* form );
static usize compile_def( chunk_t* target, list_t* form );
static usize compile_put( chunk_t* target, list_t* form );
static usize compile_lmb( chunk_t* target, list_t* form );
static usize compile_mac( chunk_t* target, list_t* form );
static usize compile_ccc( chunk_t* target, list_t* form );

chunk_t* compile( value_t src ) {
  chunk_t* target = mk_chunk(NULL, NULL, NULL, false);
  compile_expr(target, src);
  emit_instr(target, OP_RETURN);
  return target;
}

static list_t* mk_environment( list_t* parent ) {
  table_t* locals = mk_table(0, NULL);
  return cons(object(locals), parent);
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

static opcode_t resolve( value_t name, chunk_t* target, usize* i, usize* j ) {
  list_t* envt = target->envt;
  opcode_t out = OP_LOADCL;
  *i = *j = 0;
  
  for ( ; envt->arity && *i == 0; envt=envt->tail, (*j)++ ) {
    table_t* locals = as_table(envt->head);
    value_t val = table_get(locals, name);

    if ( val != NOTFOUND )
      *i = as_integer(val);
  }

  if ( *i == 0 ) {
    out = OP_LOADGL;
    *i = add_value(target, name);
  }

  return out;
}

static opcode_t extend( value_t name, chunk_t* target, usize* i ) {
  list_t* envt = target->envt;
  opcode_t out = envt->arity == 0 ? OP_PUTGL : OP_PUTCL;
  *i = 0;

  if ( out == OP_PUTGL )
    *i = add_value(target, name);

  else
    *i = as_integer(update_environment(name, envt));

  return out;
}

static value_t update_environment( value_t name, list_t* envt ) {
  table_t* locals = as_table(envt->head);
  return table_add(locals, name, locals->cnt+1);
}

static list_t* prepare_environment( list_t* parent, list_t* formals, bool* variadic ) {
  *variadic = false;
  list_t* out = mk_environment(parent);
  list_t* curr = formals;
  table_t* locals = as_table(out->head);
  value_t eformals = object(formals);

  for ( ; curr->arity; curr=curr->tail ) {
    value_t formal = curr->head;
    require("compile", is_symbol(formal), eformals, "Formal argument not a symbol");
    require("compile", !hasfl(formal, LITERAL), eformals, "Keyword arguments not yet supported");

    if ( formal == Ampersand ) {
      require("compile", curr->arity == 2, eformals, "Incorrect use of '&' syntax");
      *variadic = true;
    } else {
      require("compile", table_get(locals, formal) == NOTFOUND, eformals, "duplicate formal name");
      update_environment(formal, out);
    }
  }

  return out;
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
  usize i, j;
  opcode_t op = resolve(name, target, &i, &j);
  usize out = emit_instr(target, op, i, j);

  return out;
}


static usize compile_assignment( chunk_t* target, value_t name, value_t val ) {
  compile_expr(target, val);
  usize i, j;
  opcode_t op = resolve(name, target, &i, &j) + 1;
  return emit_instr(target, op, i, j);
}

static usize compile_definition( chunk_t* target, value_t name, value_t val ) {
  usize i;
  opcode_t op = extend(name, target, &i);

  if ( op == OP_PUTGL ) {
    usize loc = add_value(target, name);
    emit_instr(target, OP_DEFGL, loc);
    compile_expr(target, val);
    out = emit_instr(target, OP_PUTGL, loc);
  }

  else {
    compile_expr(target, val);
    out = emit_instr(target, OP_PUTCL, 0, var->offset);
  }

  return out;
}

static usize compile_combination( chunk_t* target, value_t sexpr ) {
  list_t* form = as_list(sexpr);

  if ( is_macro_call(form) ) {
    
  }

  if ( form->head == Quote )
    return compile_quote(target, form);

  if ( form->head == Do )
    return compile_do(target, form);

  if ( form->head == If )
    return compile_if(target, form);

  if ( form->head == Lmb )
    return compile_lmb(target, form);

  if ( form->head == Mac )
    return compile_mac(target, form);

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

static usize compile_closure( chunk_t* target, list_t* envt, list_t* exprs, bool variadic ) {
  chunk_t* closure = mk_chunk(envt, NULL, NULL, false);
  emit_instr(closure, variadic ? OP_VARGC : OP_ARGC, num_locals(closure));
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
  list_t* envt  = prepare_environment(target->envt, as_list(argl), &variadic);
  list_t* exprs = form->tail->tail;
  return compile_closure(target, envt, exprs, variadic);
}

static usize compile_ccc( chunk_t* target, list_t* form ) {
  require("compile", form->arity >= 3, object(form), "Incorrect arity for ccc");
  value_t ccs = form->tail->head;
  require("compile", is_symbol(ccs), object(form), "Continuation name is not a symbol");
  list_t* envt = mk_environment(target->envt);
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
    alist_t* envt = mk_alist(num_locals(f->fn) + 1, &Values[f->bp]);
    Values[f->bp] = object(envt);
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
  if ( IP == NULL ) // 
    goto op_return;

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
  for ( usize i=argx; i<num_locals(FN); i++ )
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
  argy = SP - argx - 1;
  v = as_native(x)(argx, &Values[SP-argx]);
  SP = argy;
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
  require("exec", v != UNDEFINED, x, "unbound symbol");
  goto fetch;

 op_putgl:
  x = VALS[argx];
  require("exec", table_get(&Vm.globals, x) != UNDEFINED, x, "unbound symbol");
  table_set(&Vm.globals, x, TOS);
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

// initialization +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void toplevel_init_interp( void ) {
  // create special forms & constants +++++++++++++++++++++++++++++++++++++++++
  Quote     = object(symbol("quote"));
  Do        = object(symbol("do"));
  If        = object(symbol("if"));
  Lmb       = object(symbol("lmb"));
  Mac       = object(symbol("mac"));
  Def       = object(symbol("def"));
  Put       = object(symbol("put"));
  Ccc       = object(symbol("ccc"));
  Ampersand = object(symbol("&"));
  Form      = object(symbol("&form"));
  Env       = object(symbol("&env"));
  True      = object(symbol("true"));
  False     = object(symbol("false"));

  // true and false are special cases of keywords not prefixed with ':' ------- 
  setfl(True, LITERAL);
  setfl(False, LITERAL);
}
