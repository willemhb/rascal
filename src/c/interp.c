#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "interp.h"

#include "opcodes.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// special forms --------------------------------------------------------------
value_t Quote, Do, If, Lmb, Def, Put, Ccc;

// other syntactic markers ----------------------------------------------------
value_t Ampersand, Otherwise;

// internal API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static bool is_literal( value_t x ) {
  if ( is_symbol(x) )
    return *as_symbol(x)->name == ':';

  else if ( is_list(x) )
    return as_list(x)->arity > 0;

  else return true;
}

static bool is_toplevel( void* ptr ) {
  object_t* o = ptr;

  assert(o);
  assert(o->type == ENVT || o->type == CHUNK || o->type == NS);

  return hasfl(o, TOPLEVEL);
}

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x, envt_t* envt ) {
  value_t v;

  if ( is_literal(x) )
    v = x;

  else if ( is_symbol(x) ) {
    value_t out = envt_lookup(x, envt);
    forbid("eval", out == UNDEFINED, x, "unbound symbol");
    return out;
  }

  else {
    chunk_t* chunk = compile(x, envt->ns);
    v = exec(chunk, envt);
  }

  return v;
}

void repl( void ) {
  
}

static usize emit_instr( chunk_t* target, opcode_t op, ... );
static usize add_value( chunk_t* target, value_t val );
static usize compile_value( chunk_t* target, value_t val );
static usize compile_variable( chunk_t* target, value_t name );
static usize compile_combination( chunk_t* target, value_t sexpr );
static usize compile_expr( chunk_t* target, value_t src );
static usize compile_exprs( chunk_t* target, list_t* exprs );
static usize compile_args( chunk_t* target, list_t* args );
static usize compile_quote( chunk_t* target, list_t* form );
static usize compile_do( chunk_t* target, list_t* form );
static usize compile_if( chunk_t* target, list_t* form );
static usize compile_lmb( chunk_t* target, list_t* form );
static usize compile_def( chunk_t* target, list_t* form );
static usize compile_put( chunk_t* target, list_t* form );
static usize compile_ccc( chunk_t* target, list_t* form );

chunk_t* compile( value_t src, ns_t* ns ) {
  chunk_t* target = chunk( ns );
  compile_expr(target, src);
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
  buffer_write(&target->instr, n+1, buf);
  return target->instr.cnt;
}

static usize add_value( chunk_t* target, value_t val ) {
  return values_push(&target->vals, val);
}

static usize compile_value( chunk_t* target, value_t val ) {
  usize location = add_value(target, val);
  return emit_instr(target, OP_LOADV, location);
}

static usize compile_variable( chunk_t* target, value_t name ) {
  usize i=0, j=0;

  if ( is_toplevel(target) ) {
    define(name, Vm.globals, UNDEFINED); // reserve space
  }
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

  compile_expr(target, form->head); // compile caller
  compile_args(target, form->tail); // compile arguments
  return emit_instr(target, OP_CALL, form->tail->arity);
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
  usize out = target->instr->buffer.cnt;
  
  while ( args->arity ) {
    out = compile_expr(target, args->head);
    args = args->tail;
  }

  return out;
}

static bool is_captured( frame_t* f ) {
  return !!(f->fl & CAPTURED);
}

static void capture_frame( frame_t* f ) {
  if ( !is_captured(f) ) {
    envt_t* parent = f->envt;
    ns_t* ns = f->fn->ns;
    vector_t* vals = mk_vector(ns->locals->table.cnt, &Values[f->bp]);
    f->envt = mk_envt(parent, ns, vals);
    f->fl |= CAPTURED;
  }
}

value_t apply( value_t f, value_t a );

value_t exec( value_t ch ) {
  static void* labels[] = {
    [OP_NOOP] = &&op_noop,
    [OP_START] = &&op_start,
    [OP_POP] = &&op_pop,
    [OP_CLOSURE] = &&op_closure, [OP_CONTROL] = &&op_control,
    [OP_RETURN] = &&op_return,

    [OP_ARGC] = &&op_argc, [OP_VARGC] = &&op_vargc,
    [OP_CALL] = &&op_call,
    [OP_LOADV] = &&op_loadv,
    [OP_LOADGL] = &&op_loadgl, [OP_PUTGL] = &&op_putgl,
    [OP_LOADL] = &&op_loadl, [OP_PUTL] = &&op_putl,
    [OP_JUMP] = &&op_jump, [OP_JUMPN] = &&op_jumpn,

    [OP_LOADCL] = &&op_loadcl, [OP_PUTCL] = &&op_putcl
  };

  opcode_t op;
  int argx, argy;
  value_t x, v;
  list_t* va;

 fetch: // next instruction
  op = *(IP++);

  if ( op > OP_RETURN )
    argx = *(IP++);

  if ( op > OP_JUMPN )
    argy = *(IP++);

  goto *labels[op];

 op_start:
  goto fetch;
  
 op_noop:
  goto fetch;

 op_pop:
  pop();
  goto fetch;

 op_closure:
  v = pop();
  capture_frame(&FRAME);
  v = object(mk_closure(as_chunk(v), ENV));
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

 op_argc:
  argy = SP - BP;
  require("exec", argy == argx, object(FN), "Incorrect arity");
  goto pad_locals;

 op_vargc:
  argy = SP - BP;
  require("exec", argy >= argx, object(FN), "Incorrect arity");
  va = mk_list(argy - argx, &Values[BP+argx]);
  Values[BP+argx] = object(va);
  SP = BP+argx+1;
  goto pad_locals;

 pad_locals: // adjust stack to hold all local bindings, including those introduced in function body
  for ( int i=argx; i<(int)FN->ns->locals->table.cnt; i++ )
    push(NIL);
  goto fetch;

 op_call:
  x = popnth(-argx-1);

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
  popn(argx);
  push(v);
  goto fetch;

 do_call_closure:
  push_frame();

  FN   = as_closure(x)->code;
  ENVT = as_closure(x)->envt;
  IP   = FN->instr->buffer.data;
  BP   = SP - argx;
  FL   = 0;
  
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
  push(GLOBALS[argx]);
  goto fetch;

 op_putgl:
  GLOBALS[argx] = Values[SP-1];
  goto fetch;
  
 op_jump:
  IP += argx;
  goto fetch;

 op_jumpn:
  x = pop();

  if ( x == NIL )
    IP += argx;

  goto fetch;

 op_loadcl:{
    envt_t* e = ENV;

    if ( !is_captured(&FRAME) )
      argx--;
    
    while ( argx-- )
      e = e->next;
    
    push(e->binds->values.data[argy]);
  }
  
  goto fetch;

 op_putcl:{
    envt_t* e = ENV;

    if ( !is_captured(&FRAME) )
      argx--;
    
    while ( argx-- )
      e = e->next;

    e->binds->values.data[argy] = Values[SP-1];
  }
  
  goto fetch;

 op_loadl:
  if ( is_captured(&FRAME) )
    v = ENV->binds->values.data[argx];

  else
    v = Values[BP+argx];

  push(v);

  goto fetch;

 op_putl:
  if ( is_captured(&FRAME) )
    ENV->binds->values.data[argx] = Values[SP-1];

  else
    Values[BP+argx] = Values[SP-1];

  goto fetch;
}
