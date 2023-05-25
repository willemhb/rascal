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

void push_frame( void ) {
  *(Vm.frame.fp++) = Vm.frame;
}

void pop_frame( void ) {
  Vm.frame = *Vm.frame.cp;
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

value_t* frame_locals( frame_t* frame ) {
  return is_captured( frame ) ? frame->envt->data : frame->bp;
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

value_t apply( void* head, usize n, value_t* args );

value_t exec( chunk_t* chunk ) {
  static void* labels[] = {
    [OP_NOOP]       =&&op_noop,

    [OP_LOAD_VALUE] =&&op_load_value,
    [OP_LOAD_LOCAL] =&&op_load_local,  [OP_PUT_LOCAL] =&&op_put_local,
    [OP_LOAD_GLOBAL]=&&op_load_global, [OP_PUT_GLOBAL]=&&op_put_global,

    [OP_JUMP]       =&&op_jump,        [OP_JUMP_NIL]  =&&op_jump_nil,

    [OP_CLOSURE]    =&&op_closure,     [OP_CALL]      =&&op_call,
    [OP_RETURN]     =&&op_return
  };

  value_t x, v=NIL, *a;
  frame_t* f;
  vector_t* e;
  opcode_t op;
  int argx=0, argy=0, argc=0;

  Vm.frame.code = chunk;
  Vm.frame.ip = chunk->code->data;

 dispatch:
  op = *Vm.frame.ip++;
  argc = opcode_argc(op);

  if ( argc > 0 )
    argx = *Vm.frame.ip++;

  if ( argc > 1 )
    argy = *Vm.frame.ip++;

  goto *labels[op];

 op_noop:
  goto dispatch;

 op_load_value:
  push( Vm.frame.code->vals->data[argx] );
  goto dispatch;

 op_load_local:
  a = frame_locals( &Vm.frame );
  while ( argy-- )
    a = as_vector(a[0])->data;
  push( a[argx] );
  goto dispatch;

 op_put_local:
  a = frame_locals( &Vm.frame );
  while ( argy-- )
    a = as_vector(a[0])->data;
  a[argx] = Vm.frame.sp[-1];
  goto dispatch;
  
 op_load_global:
  push( Vm.global.data[argx*2+1] );
  goto dispatch;

 op_put_global:
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
  f = &Vm.frame;

  while ( f ) {
    f = f->cp;
  }
  
 op_call:
  x = Vm.frame.sp[-argx-1];

 call_native:

 call_closure:

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
  
}
