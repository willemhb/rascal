#include <stdlib.h>

#include "memory.h"
#include "opcodes.h"
#include "function.h"
#include "array.h"
#include "rlvm.h"

value_t rl_exec( closure_t *module )
{
  static void *labels[] =
    {
      [op_invalid]      =&&label_invalid,
      [op_nothing]      =&&label_nothing,
      
      [op_load_nul]     =&&label_load_nul,
      [op_load_true]    =&&label_load_true,
      [op_load_false]   =&&label_load_false,

      [op_load_value]   =&&label_load_value,
      [op_load_const]   =&&label_load_const,

      [op_load_local]   =&&label_load_local,
      [op_store_local]  =&&label_store_local,
      [op_load_global]  =&&label_load_global,
      [op_store_global] =&&label_store_global,
      [op_load_closure] =&&label_load_closure,
      [op_store_closure]=&&label_store_closure,

      [op_jump_true]    =&&label_jump_true,
      [op_jump_false]   =&&label_jump_false,
      [op_jump]         =&&label_jump,

      [op_invoke]       =&&label_invoke,
      [op_return]       =&&label_return,
      [op_closure]      =&&label_closure,

      [op_argco]        =&&label_argco,
      [op_vargco]       =&&label_vargco,

      [op_halt]         =&&label_halt
    };

  ushort op = 0;

  size_t argc;
  
  short  argx, argy;

  value_t x, y;

  control_t *control = make_control( module, NULL );
  environment_t *eframe;

 label_fetch:
  op   = *(control->ip++);

  argc = op_argc(op);

  if (argc > 0)
    argx = *(control->ip++);

  if (argc > 1)
    argy = *(control->ip++);

  goto *labels[op];

 label_invalid:
  fprintf(stderr, "error: invalid opcode %d.\n", op);
  fprintf(stderr, "aborting.\n");
  abort();

 label_nothing:

  goto label_fetch;

 label_load_nul:
  rl_push( NUL );

  goto label_fetch;

 label_load_true:
  rl_push( TRUE );

  goto label_fetch;

 label_load_false:
  rl_push( FALSE );

  goto label_fetch;

 label_load_value:
  x = control->function->constants->data[argx];
  
  rl_push( x );
  
  goto label_fetch;

 label_load_const:
  x = control->function->constants->data[argx];
  x = as_atom(x)->constant;

  rl_push( x );

  goto label_fetch;

 label_load_local:
  x = control->locals[argx];
  
  rl_push( x );

  goto label_fetch;

 label_store_local:
  x                     = rl_peek( 1 );
  control->locals[argx] = x;

  goto label_fetch;

 label_load_global:
  y = control->function->constants->data[argx];
  x = as_atom(y)->bind;

  rl_push(x);

  goto label_fetch;

 label_store_global:
  y                = control->function->constants->data[argx];
  as_atom(y)->bind = rl_peek(1);

  goto label_fetch;

 label_load_closure:
  eframe = control->environment;

  while (argx--)
    eframe = eframe->next;

  x = eframe->binds->data[argy];

  rl_push(x);

  goto label_fetch;

 label_store_closure:
  eframe = control->environment;

  while (argx--)
    eframe = eframe->next;

  eframe->binds->data[argy] = rl_peek(1);

  goto label_fetch;
}
