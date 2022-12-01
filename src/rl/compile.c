#include "obj/lambda.h"
#include "obj/symbol.h"
#include "obj/cons.h"

#include "vm/error.h"
#include "vm/memory.h"

#include "rl/compile.h"
#include "rl/eval.h"        // for is_literal()

#include "rl/stx/if.h"
#include "rl/stx/quote.h"
#include "rl/stx/fun.h"
#include "rl/stx/var.h"
#include "rl/stx/do.h"

/* commentary */

/* C types */

/* globals */

/* API */
int compile_expression( lambda_t *lambda, value_t x );
int compile_combination( lambda_t *lambda, cons_t *x );
int compile_variable( lambda_t *lambda, value_t x );
int compile_literal( lambda_t *lambda, value_t x );

int compile_funcall( lambda_t *lambda, cons_t *x );
int compile_quote( lambda_t *lambda, cons_t *x );
int compile_if( lambda_t *lambda, cons_t *x );
int compile_do( lambda_t *lambda, cons_t *x );
int compile_var( lambda_t *lambda, cons_t *x );
int compile_fun( lambda_t *lambda, cons_t *x );

int compile_args( lambda_t *lambda, value_t args );
int compile_sequence( lambda_t *lambda, value_t x );


lambda_t *compile( value_t x )
{
  lambda_t *out;

  /* many reallocations could happen during compilation, so preserve these at top level */
  save_values(1, &x);
  save_objects(1, (object_t**)&out);

  out = make_lambda();

  if ( compile_expression(out, x) < 0 )
    return NULL;

  finalize_lambda_const(out);
  finalize_lambda_instr(out);

  return out;
}

int compile_expression( lambda_t *lambda, value_t x )
{
  if ( is_symbol(x) )
    return compile_variable(lambda, x);

  else if ( is_cons(x) )
    return compile_combination(lambda, as_cons(x));

  else
    return compile_literal(lambda, x);
}

int compile_literal( lambda_t *lambda, value_t x )
{
  size_t loc = put_lambda_const(lambda, x);

  return emit_lambda_instr(lambda, op_load_const, loc);
}

int compile_variable( lambda_t *lambda, value_t x )
{
  size_t loc = put_lambda_const(lambda, x);

  return emit_lambda_instr(lambda, op_load_global, loc);
}

int compile_combination( lambda_t *lambda, cons_t *form )
{
  if ( is_quote_form(form) )
    return compile_quote(lambda, form);

  if ( is_if_form(form) )
    return compile_if(lambda, form);

  if ( is_do_form(form) )
    return compile_do(lambda, form);

  if ( is_var_form(form) )
    return compile_var(lambda, form);

  if ( is_fun_form(form) )
    return compile_fun(lambda, form);

  return compile_funcall(lambda, form);
}

int compile_funcall( lambda_t *lambda, cons_t *x )
{
  value_t f = get_cons_car(x);
  value_t a = get_cons_cdr(x);

  int status;

  if ( (status=compile_expression(lambda, f)) < 0 )
    return status;

  if ( (status=compile_args(lambda, a)) < 0 )
    return status;
  
  return emit_lambda_instr(lambda, op_invoke, status);
}

int compile_quote( lambda_t *lambda, cons_t *form )
{
  value_t x;
  int status;

  if ( (status=check_quote_syntax(form)) < 0 )
    return status;

  x = quote_quoted(form, status);
  
  return compile_literal(lambda, x);
}

int compile_if( lambda_t *lambda, cons_t *form )
{
  int status = check_if_syntax(form);

  if (status < 0)
    return status;

  value_t test = if_test(form, status),
    then = if_then(form, status),
    otherwise = if_otherwise(form, status);
  
  if ( (status=compile_expression(lambda, test)) < 0 )
    return status;

  /* save location of the jump argument to fill in once we know the offset */
  int test_fill = emit_lambda_instr(lambda, op_jump_false, 0) - 1;

  if ( (status=compile_expression(lambda, then)) < 0 )
    return status;

  /* save location of the jump argument to fill in once we know the offset */
  int then_fill = emit_lambda_instr(lambda, op_jump, 0) - 1;

  if ( (status=compile_expression(lambda, otherwise)) > -1 )
    {
      fill_lambda_instr(lambda, test_fill, then_fill-test_fill);
      fill_lambda_instr(lambda, then_fill, status-then_fill);
    }

  return status;
}

int compile_do( lambda_t *lambda, cons_t *form )
{
  int status = check_do_syntax(form);

  if ( status < 0 )
    return status;
  
  return compile_sequence(lambda, do_sequence(form, status));
}

int compile_args( lambda_t *lambda, value_t args )
{
  int n = 0, status = 0;

  while ( is_cons(args) )
    {
      value_t x = get_cons_car(as_cons(args));

      if ((status=compile_expression(lambda, x)))
	return status;

      n   += 1;
      args = get_cons_cdr(as_cons(args));
    }

  return n;
}

int compile_sequence( lambda_t *lambda, value_t seq )
{
  int status;

  while ( is_cons(seq) )
    {
      value_t x = get_cons_car(as_cons(seq));

      if ( (status=compile_expression(lambda, x)) < 0 )
	return status;

      seq = get_cons_cdr(as_cons(seq));

      if ( is_cons(seq) )
	emit_lambda_instr(lambda, op_pop);
    }

  return get_instr_len(lambda);
}

/* runtime */
void rl_rl_compile_init( void ) {}
void rl_rl_compile_mark( void ) {}
void rl_rl_compile_cleanup( void ) {}

/* convenience */
