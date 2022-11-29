#include "obj/lambda.h"

#include "vm/error.h"
#include "vm/obj/heap.h"

#include "rl/compile.h"
#include "rl/eval.h"        // for is_literal()

/* commentary */

/* C types */

/* globals */

/* API */
void compile_expression( lambda_t *lambda, value_t x );
void compile_literal( lambda_t *lambda, value_t x );

lambda_t *compile( value_t x )
{
  save_vals(1, x);
  lambda_t *out = make_lambda();
  save_objs(1, &out->obj);
  compile_expression(out, x);
  finalize_lambda(out);
  
  unsave_objs(1);
  unsave_vals(1);

  return out;
}

void compile_expression( lambda_t *lambda, value_t x )
{
  if ( is_literal(x) )
    compile_literal(lambda, x);

  else
      panic("Unknown expression type.");
}

void compile_literal( lambda_t *lambda, value_t x )
{
  size_t loc = put_const(lambda, x);

  emit_instr(lambda, op_load_const, loc);
}

/* runtime */
void rl_rl_compile_init( void ) {}
void rl_rl_compile_mark( void ) {}

/* convenience */
