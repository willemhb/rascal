#include "eval.h"
#include "compile.h"
#include "read.h"
#include "print.h"
#include "runtime.h"
#include "object.h"
#include "opcodes.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define NSTACK 32768
#define NFRAME 2048
#define StackEnd (Stack+NSTACK)
#define FrameEnd (Frame+NFRAME)

value_t Stack[NSTACK];
frame_t Frame[NFRAME];

struct Vm Vm = {
  .frame ={
    .cp  =NULL,
    .fp  =Frame,
    .code=NULL,
    .bp  =Stack,
    .sp  =Stack,
    .ip  =NULL
  },
  .global={
    .obj ={
      .next =NULL,
      .type =TABLE,
      .flags=NOFREE|GRAY|IDTABLE
    },
    .cnt =0,
    .cap =0,
    .data=NULL,
    .ord =NULL
  }
};

// internal API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void reset_vm( struct Vm* vm ) {
  vm->frame.cp   = NULL;
  vm->frame.fp   = Frame;
  vm->frame.code = NULL;
  vm->frame.sp   = Stack;
  vm->frame.bp   = Stack;
  vm->frame.ip   = NULL;
}

value_t* push( value_t x ) {
  value_t* out = Vm.frame.sp;
  *(Vm.frame.sp++) = x;
  return out;
}

value_t pop( void ) {
  return *(--Vm.frame.sp);
}

value_t popn( usize n ) {
  value_t out  = Vm.frame.sp[-n];
  Vm.frame.sp -= n;
  return out;
}

void push_frame( int nargs, closure_t* cl ) {
  frame_t* current = &Vm.frame;
  *(Vm.frame.fp++) = Vm.frame;
  Vm.frame.cp      = current;
  Vm.frame.bp      = Vm.frame.sp - nargs - 1;
  chunk_t* cnk     = cl->code;
  int argco        = cnk->obj.flags & UINT16_MAX;

  if ( has_flag(chunk, VARIADIC) ) {
    int      nv = nargs - argco;
    value_t* vb = Vm.frame.sp - nv;
    list_t*  va = list(nv, vb);
    popn(nv);
    push(object(va));
  }

  table_t* locals = as_table(cnk->envt->head);
  int      nenv   = locals->cnt;
  int      ninner = nenv - argco;

  for ( int i=0; i<ninner; i++)
    push(UNDEFINED);

  Vm.frame.bp[0] = object(cl->envt);
}

void pop_frame( void ) {
  Vm.frame    = *Vm.frame.cp;
}

bool is_literal( value_t x ) {
  if ( is_symbol(x) )
    return has_flag(as_symbol(x), LITERAL);

  else if ( is_list(x) )
    return as_list(x)->arity > 0;

  else
    return true;
}

bool is_captured( frame_t* frame ) {
  return frame->bp < Stack || frame->bp >= StackEnd;
}

bool is_function( value_t x ) {
  type_t t = type_of(x);

  return t == CLOSURE || t == NATIVE;
}

bool is_variadic( value_t x ) {
  return is_function(x) && has_flag(as_object(x), VARIADIC);
}

usize fn_argco( value_t f ) {
  assert(is_function(f));

  return as_object(f)->flags & UINT16_MAX;
}

value_t* frame_locals( frame_t* frame ) {
  return is_captured( frame ) ? frame->envt->data : frame->bp;
}

char* fname( void ) {
  return Vm.frame.code->name->name;
}

symbol_t* local_name( int i, int j ) {
  list_t* lenv = Vm.frame.code->envt;

  for ( int n=0; n<i; n++ )
    lenv = lenv->tail;

  table_t* locals = as_table(lenv->head);

  assert(j < (int)locals->cnt);
  return as_symbol(locals->data[j*2]);
}

symbol_t* global_name( int j ) {
  return as_symbol(Vm.global.data[j*2]);
}

vector_t* capture_frame( frame_t* frame ) {
  if ( !is_captured(frame) ) {
    table_t* locals = as_table(frame->code->envt->head);
    int nlocals     = locals->cnt;
    frame->envt     = vector(nlocals+1, frame->bp);
  }
 
  return frame->envt;
}


value_t do_exec( chunk_t* chunk, opcode_t op, int argx, int argy ) {
  static void* labels[] = {
    [OP_NOOP]       =&&op_noop,
    [OP_START]      =&&op_start,

    [OP_LOAD_VALUE]=&&op_load_value, [OP_LOAD_NIL]   =&&op_load_nil,
    [OP_LOAD_LIST0]=&&op_load_list0,

    [OP_DEF_LOCAL] =&&op_def_local,  [OP_LOAD_LOCAL] =&&op_load_local,
    [OP_PUT_LOCAL] =&&op_put_local,

    [OP_DEF_GLOBAL]=&&op_def_global, [OP_LOAD_GLOBAL]=&&op_load_global,
    [OP_PUT_GLOBAL]=&&op_put_global,

    [OP_JUMP]      =&&op_jump,        [OP_JUMP_NIL]  =&&op_jump_nil,

    [OP_CLOSURE]   =&&op_closure,     [OP_CALL]      =&&op_call,
    [OP_RETURN]    =&&op_return
  };

  value_t x, v=NIL, *a;
  vector_t* e;
  symbol_t* s;
  int argz=0, argc=0;

  goto *labels[op];

 dispatch:
  op = *Vm.frame.ip++;
  argc = opcode_argc(op);

  if ( argc > 0 )
    argx = *Vm.frame.ip++;

  if ( argc > 1 )
    argy = *Vm.frame.ip++;

  goto *labels[op];

 op_start:
  Vm.frame.code = chunk;
  Vm.frame.ip = chunk->code->data;

 op_noop:
  goto dispatch;

 op_load_value:
  push(Vm.frame.code->vals->data[argx]);
  goto dispatch;

 op_load_nil:
  push(NIL);
  goto dispatch;

 op_load_list0:
  push(object(&EmptyList));
  goto dispatch;

 op_def_local:
  a = frame_locals(&Vm.frame);
  while ( argy-- )
    a = as_vector(a[0])->data;

  if ( a[argx+1] == UNDEFINED )
    a[argx] = NIL;

  goto dispatch;

 op_load_local:
  a    = frame_locals(&Vm.frame);
  argz = argy;
  while ( argz-- )
    a = as_vector(a[0])->data;
  v = a[argx+1];

  if ( v == UNDEFINED ) {
    s = local_name(argy, argx);
    error(fname(), object(s), "undefined symbol '%s'", s->name);
  }

  push(v);
  goto dispatch;

 op_put_local:
  a    = frame_locals( &Vm.frame );
  argz = argy;
  while ( argz-- )
    a = as_vector(a[0])->data;

  if ( a[argx] == UNDEFINED ) {
    s = local_name(argy, argx);
    error(fname(), object(s), "undefined symbol '%s'", s->name);
  }
  
  a[argx+1] = Vm.frame.sp[-1];
  goto dispatch;

 op_def_global:
  if ( Vm.global.data[argx*2+1] == UNDEFINED )
    Vm.global.data[argx*2+1] = NIL;
  goto dispatch;
  
 op_load_global:
  v = Vm.global.data[argx*2+1];

  if ( v == UNDEFINED ) {
    s = global_name(argx);
    error(fname(), object(s), "undefined symbol '%s'", s->name);
  }

  push(v);
  goto dispatch;

 op_put_global:
  if ( Vm.global.data[argx*2+1] == UNDEFINED ) {
    s = global_name(argx);
    error(fname(), object(s), "undefined symbol '%s'", s->name);
  }

  Vm.global.data[argx*2+1] = Vm.frame.sp[-1];
  goto dispatch;

 op_jump:
  Vm.frame.ip += argx;
  goto dispatch;

 op_jump_nil:
  x = pop();
  if ( x == NIL )
    Vm.frame.ip += argx;
  goto dispatch;

 op_closure:
  e = capture_frame(&Vm.frame);
  x = pop();
  v = object(closure(as_chunk(x), e));
  push(v);
  goto dispatch;
  
 op_call:
  x = Vm.frame.sp[-argx-1];
  require("exec", is_function(x), x, "Not a function");
  argc = fn_argco(x);

  if ( is_variadic(x) )
    require("exec", argc >= argx, x, "incorrect arity: expected at least %d, got %d", argc, argx);

  else
    require("exec", argc == argx, x, "incorrect arity: expected %d, got %d", argc, argx);

  if ( is_closure(x) )
    goto call_closure;

  else
    goto call_native;

 call_native:
  v = as_native(x)->native(argx, Vm.frame.sp-argx);
  popn(argx+1);
  push(v);
  goto dispatch;

 call_closure:
  push_frame(argx, as_closure(x));
  goto dispatch;

 op_return:
  v = pop();
  if ( Vm.frame.cp == NULL ) {
    reset_vm(&Vm);
    return v;
  }

  pop_frame();
  push(v);
  goto dispatch;
}

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x ) {
  value_t v;
  
  if ( is_literal(x) )
    v = x;

  else if ( is_symbol(x) ) {
    v = table_get(&Vm.global, x);
    forbid("eval", v==NOTFOUND, x, "symbol undefined at toplevel");
  } else {
    list_t* form = as_list(x);
    chunk_t* chunk = compile(form);
    v = exec(chunk);
  }

  return v;
}

value_t exec( chunk_t* chunk ) {
  return do_exec(chunk, OP_START, 0, 0);
}

#define PROMPT "rascal>"

void repl( void ) {
  for (;;) {
    value_t x, v;
    printf("\n"PROMPT" ");

    if ( setjmp(Error.safety) ) {
      recover();
      printf("\n");
      continue;

    } else {
      x = read(stdin);
      v = eval(x);
      fprintf(stdout, "\n");
      print(stdout, v);
    }
  }
}

void vm_init( void ) {
  reset_table(&Vm.global);
}
