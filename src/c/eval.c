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
control_t Frame[NFRAME];

struct Vm Vm = {
  .fp=Frame,
  .sp=Stack,
  .globals={
    .obj ={
      .next =NULL,
      .type =TABLE,
      .flags=GRAY|IDTABLE
    },
    .cnt =0,
    .dcap=0,
    .ocap=0,
    .data=NULL,
    .ord =NULL
  }
};

static void init_frames( void ) {
  for ( usize i=0; i<NFRAME; i++ ) {
    Frame[i] = (control_t) {
      .obj  ={ .type =CONTROL },
      .cp   =Frame,
      .stack=NULL
    };
  }
}

// internal API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t do_exec( function_t* chunk, opcode_t op, int argx, int argy ) {
  static void* labels[] = {
    [OP_NOOP] =&&op_noop,
    
    [OP_SYMBOL] =&&op_symbol, [OP_CONS] =&&op_cons,  [OP_BINARY] =&&op_binary,
    [OP_VECTOR] =&&op_vector, [OP_TABLE]=&&op_table, []
  };

 fetch:
  
 op_noop:
  goto fetch;

 op_symbol:
  
 op_cons:

 op_binary:

 op_vector:
  
}

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x ) {
  value_t v;
  
  if ( is_literal(x) )
    v = x;

  else if ( is_symbol(x) ) {
    v = table_get(&Vm.globals, x);
    forbid("eval", v==NOTFOUND, x, "symbol undefined at toplevel");
  } else {
    cons_t* form = as_cons(x);
    function_t* chunk = compile(form);
    v = exec(chunk);
  }

  return v;
}

value_t exec( function_t* chunk ) {
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
  reset_table(&Vm.globals);
  init_frames();
}
