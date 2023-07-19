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

// metadata keywords ----------------------------------------------------------
value_t Macro;

// special constants ----------------------------------------------------------
value_t True, False;

// toplevel value of eval/compile variables -----------------------------------
variable_t* Eval, * Compile;

#define do_eval( x )                            \
  ({                                            \
    push(Eval->binding);                        \
    push(x);                                    \
    do_exec(NULL, OP_APPLY);                    \
  })

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
  return is_primitive(x) || is_native(x) || is_closure(x);
}

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x ) {
  value_t v;

  if ( is_literal(x) )
    v = x;

  else if ( is_symbol(x) ) {
    v = table_get(&Vm.globals, x);
    forbid("eval", v == NOTFOUND, x, "unbound symbol");
    v = as_variable(v)->binding;
    forbid("eval", v == UNDEFINED, x, "unbound symbol");
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
static list_t* mk_environment( list_t* parent );
static usize emit_instr( chunk_t* target, opcode_t op, ... );
static void fill_jump( chunk_t* target, usize offset );
static usize add_value( chunk_t* target, value_t val );
static variable_t* resolve( value_t name, list_t* envt );
static variable_t* extend( value_t name, list_t* envt, bool allowDuplicate );
static list_t* prepare_environment( list_t* parent, list_t* formals, bool* variadic );
static usize compile_value( chunk_t* target, value_t val );
static usize compile_variable( chunk_t* target, value_t name );
static usize compile_combination( chunk_t* target, value_t sexpr );
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
static usize compile_ccc( chunk_t* target, list_t* form );

chunk_t* compile( value_t src ) {
  chunk_t* target = mk_chunk( NULL, NULL, NULL );
  compile_expr(target, src);
  emit_instr(target, OP_RETURN);
  return target;
}

static list_t* mk_environment( list_t* parent ) {
  table_t* locals = mk_table(0, NULL);
  return list(object(locals), parent);
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

static variable_t* resolve( value_t name, list_t* envt ) {
  variable_t* var = NULL;

  for ( ; envt->arity && var == NULL; envt=envt->tail ) {
    table_t* locals = as_table(envt->head);
    value_t val = table_get(locals, name);

    if ( val != NOTFOUND )
      var = as_variable(val);
  }

  if ( var == NULL ) {
    value_t global = table_get(&Vm.globals, name);

    if ( global != NOTFOUND )
      var = as_variable(global);

    else {
      var = mk_global_variable(as_symbol(name), UNDEFINED);
      table_set(&Vm.globals, name, object(var));
    }
  }

  return var;
}

static variable_t* extend( value_t name, list_t* envt, bool allowDuplicate ) {
  bool toplevel = envt->arity == 0;
  variable_t* var = NULL;

  table_t* vars = toplevel ? as_table(envt->head) : &Vm.globals;
  value_t val = table_get(vars, name);

  if ( val != NOTFOUND ) {
    require("compile", allowDuplicate, name, "name already exists in given environment");
    var = as_variable(val);
  }

  else {
    if ( toplevel )
      var = mk_global_variable(as_symbol(name), UNDEFINED);
    else
      var = mk_local_variable(as_symbol(name), envt->arity, vars->cnt+1);

    table_set(vars, name, object(var));
  }

  return var;
}

static list_t* prepare_environment( list_t* parent, list_t* formals, bool* variadic ) {
  *variadic = false;
  list_t* out = mk_environment(parent);
  list_t* curr = formals;

  for ( ; curr->arity; curr=curr->tail ) {
    value_t formal = curr->head;
    require("compile", is_symbol(formal), object(formals), "Formal argument not a symbol");
    require("compile", !hasfl(formal, LITERAL), object(formals), "Keyword arguments not yet supported");

    if ( formal == Ampersand ) {
      require("compile", curr->arity == 2, object(formals), "Incorrect use of '&' syntax");
      *variadic = true;
    } else {
      extend(formal, out, false);
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
  variable_t* var = resolve(name, target->envt);
  usize out;

  if ( hasfl(var, TOPLEVEL) ) {
    usize loc = add_value(target, object(var));
    out = emit_instr(target, OP_LOADGL, loc);
  }

  else {
    usize i = target->envt->arity - var->depth, j = var->offset;
    out = emit_instr(target, OP_LOADCL, i, j);
  }

  return out;
}


static usize compile_assignment( chunk_t* target, value_t name, value_t val ) {
  compile_expr(target, val);
  variable_t* var = resolve(name, target->envt);
  usize out;

  if ( hasfl(var, TOPLEVEL) ) {
    usize loc = add_value(target, object(var));
    out = emit_instr(target, OP_PUTGL, loc);
  } else {
    usize i = target->envt->arity - var->depth, j = var->offset;
    out = emit_instr(target, OP_PUTCL, i, j);
  }

  return out;
}

static usize compile_definition( chunk_t* target, value_t name, value_t val ) {
  usize out;
  variable_t* var = extend(name, target->envt, true);

  if ( hasfl(var, TOPLEVEL) ) {
    usize loc = add_value(target, object(var));
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

static usize compile_closure( chunk_t* target, list_t* envt, list_t* exprs, bool variadic ) {
  chunk_t* closure = mk_chunk(envt, NULL, NULL);
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
  extend(ccs, envt, false);
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

    [OP_APPLY] = &&op_apply,
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

 op_apply:
  argco("apply", false, argx, 2);
  x  = Values[SP-argx];
  va = to_list("apply", Values[SP-argx+1]);
  Values[SP-argx-1] = x;
  SP = SP - 2;

  for ( ; va->arity; va=va->tail )
    push(va->head);

  goto op_call;

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
  if ( is_primitive(x) )
    goto do_call_primitive;
  if ( is_native(x) )
    goto do_call_native;
  else if ( is_closure(x) )
    goto do_call_closure;
  else if ( is_control(x) )
    goto do_call_control;
  else
    error("exec", x, "not a function");

 do_call_primitive:
  goto *labels[as_primitive(x)];

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
  v = as_variable(x)->binding;
  require("exec", v != UNDEFINED, x, "unbound symbol");
  goto fetch;

 op_putgl:
  x = VALS[argx];
  require("exec", as_variable(x)->binding != UNDEFINED, x, "unbound symbol");
  as_variable(x)->binding = TOS;
  goto fetch;

 op_defgl:
  x = VALS[argx];
  if ( as_variable(x)->binding == UNDEFINED )
    as_variable(x)->binding = NIL;
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
  Def       = object(symbol("def"));
  Put       = object(symbol("put"));
  Ccc       = object(symbol("ccc"));
  Ampersand = object(symbol("&"));
  True      = object(symbol("true"));
  False     = object(symbol("false"));

  // true and false are special cases of keywords not prefixed with ':' ------- 
  setfl(True, LITERAL);
  setfl(False, LITERAL);
}
