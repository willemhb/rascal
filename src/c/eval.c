#include "eval.h"
#include "compile.h"
#include "read.h"
#include "print.h"
#include "runtime.h"
#include "object.h"
#include "opcodes.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define NSTACK 32768
#define STACKEND (Stack+NSTACK)

value_t Stack[NSTACK];

struct Vm Vm = {
  .code     =NULL,
  .envt     =NULL,
  .ip       =NULL,
  .bp       =Stack,
  .cp       =Stack,
  .sp       =Stack,

  .toplevel ={
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
value_t* push( value_t x ) {
  value_t* out = Vm.sp;
  *(Vm.sp++) = x;
  return out;
}

value_t pop( void ) {
  return *(--Vm.sp);
}

void push_frame( void ) {
  push(object(Vm.code));
  push(object(Vm.envt));
  push(pointer(Vm.ip));
  push(pointer(Vm.bp));
  push(pointer(Vm.cp));
  Vm.cp = Vm.sp;
}

void pop_frame( void ) {
  Vm.sp   = Vm.cp;
  Vm.cp   = as_pointer(pop());
  Vm.bp   = as_pointer(pop());
  Vm.ip   = as_pointer(pop());
  Vm.envt = as_vector(pop());
  Vm.code = as_chunk(pop());
}

bool is_literal( value_t x ) {
  if ( is_symbol(x) )
    return has_flag(as_symbol(x), LITERAL);

  else if ( is_list(x) )
    return as_list(x)->arity > 0;

  else
    return true;
}

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x ) {
  value_t v;
  
  if ( is_literal(x) )
    v = x;

  else if ( is_symbol(x) ) {
    v = table_get(&Vm.toplevel, x);
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
    [OP_ARGCO]      =&&op_argco,       [OP_VARGCO]    =&&op_vargco,

    [OP_LOAD_VALUE] =&&op_load_value,
    [OP_LOAD_LOCAL] =&&op_load_local,  [OP_PUT_LOCAL] =&&op_put_local,
    [OP_LOAD_GLOBAL]=&&op_load_global, [OP_PUT_GLOBAL]=&&op_put_global,

    [OP_JUMP]       =&&op_jump,        [OP_JUMP_NIL]  =&&op_jump_nil,

    [OP_CLOSURE]    =&&op_closure,     [OP_CALL]      =&&op_call,
    [OP_RETURN]     =&&op_return
  };

  value_t x, v=NIL, *a;
  opcode_t op;
  int argx, argy, argc, nargs=0;

 dispatch:
  op = *Vm.ip++;
  argc = opcode_argc(op);

  if ( argc > 0 )
    argx = *Vm.ip++;

  if ( argc > 1 )
    argy = *Vm.ip++;

  goto *labels[op];
  
 op_noop:
  goto dispatch;

 op_argco:
  require( Vm.code->name->name,
           nargs == argx,
           NIL,
           "incorrect arity: expected %d, got %d",
           argx,
           nargs );
  goto dispatch;
  
 op_vargco:
    require( Vm.code->name->name,
           nargs >= argx,
           NIL,
           "incorrect arity: expected at least %d, got %d",
           argx,
           nargs );
  goto dispatch;
  
 op_load_value:
  push( Vm.code->vals->data[argx] );
  goto dispatch;
  
 op_load_local:
  a = Vm.bp;
  while ( argy-- )
    a = as_vector(a[0])->data;
  push( a[argx] );
  goto dispatch;

 op_put_local:
  a = Vm.bp;
  while ( argy-- )
    a = as_vector(a[0])->data;
  a[argx] = Vm.sp[-1];
  goto dispatch;
  
 op_load_global:
  push( Vm.toplevel.data[argx*2+1] );
  goto dispatch;

 op_put_global:
  Vm.toplevel.data[argx*2+1] = Vm.sp[-1];
  goto dispatch;

 op_jump:
  Vm.ip += argx;
  goto dispatch;
  
 op_jump_nil:
  x = pop();
  if ( x == NIL )
    Vm.ip += argx;
  goto dispatch;
  
 op_closure:
 op_call:
 op_return:
  return v;
}

void repl( void );
