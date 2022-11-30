#include "obj/lambda.h"
#include "obj/symbol.h"
#include "obj/cons.h"

#include "vm/error.h"
#include "vm/obj/heap.h"

#include "rl/compile.h"
#include "rl/eval.h"        // for is_literal()

/* commentary */

/* C types */

/* globals */

/* API */
void   compile_expression( lambda_t *lambda, value_t x );
void   compile_funcall( lambda_t *lambda, value_t x );
void   compile_variable( lambda_t *lambda, value_t x );
void   compile_literal( lambda_t *lambda, value_t x );
size_t compile_sequence( lambda_t *lambda, value_t x, opcode_t accum );

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
  if ( is_symbol(x) )
    compile_variable(lambda, x);

  else if ( is_cons(x) )
    compile_funcall(lambda, x);

  else
    compile_literal(lambda, x);
}

void compile_literal( lambda_t *lambda, value_t x )
{
  size_t loc = put_const(lambda, x);

  emit_instr(lambda, op_load_const, loc);
}

void compile_variable( lambda_t *lambda, value_t x )
{
  size_t loc = put_const(lambda, x);

  emit_instr(lambda, op_load_global, loc);
}

void compile_funcall( lambda_t *lambda, value_t x )
{
  value_t f = as_cons(x)->car;
  value_t a = as_cons(x)->cdr;

  compile_expression(lambda, f);

  size_t n = compile_sequence(lambda, a, op_nothing);
  emit_instr(lambda, op_invoke, n);
}

size_t compile_sequence( lambda_t *lambda, value_t a, opcode_t accumulate )
{
  size_t n;

  for ( n=0 ;is_cons(a); n++ )
    {
      value_t x = as_cons(a)->car;
      compile_expression(lambda, x);

      if ( accumulate != op_nothing )
	emit_instr(lambda, accumulate);

      a = as_cons(a)->cdr;
    }

  return n;
}

/* runtime */
void rl_rl_compile_init( void ) {}
void rl_rl_compile_mark( void ) {}

/* convenience */
