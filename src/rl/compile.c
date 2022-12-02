#include <errno.h>
#include <string.h>

#include "obj/lambda.h"
#include "obj/symbol.h"
#include "obj/cons.h"
#include "obj/stream.h"

#include "vm/error.h"
#include "vm/memory.h"

#include "rl/compile.h"
#include "rl/read.h"

#include "rl/stx/if.h"
#include "rl/stx/quote.h"
#include "rl/stx/fun.h"
#include "rl/stx/val.h"
#include "rl/stx/do.h"
#include "rl/stx/def.h"

/* commentary */

/* C types */

/* globals */
symbol_t *Toplevel;

/* local helper declarations */
int compile_expression( lambda_t *lambda, value_t x );
int compile_combination( lambda_t *lambda, cons_t *x );
int compile_variable( lambda_t *lambda, value_t x );
int compile_literal( lambda_t *lambda, value_t x );

int compile_funcall( lambda_t *lambda, cons_t *x );
int compile_quote( lambda_t *lambda, cons_t *x );
int compile_if( lambda_t *lambda, cons_t *x );
int compile_do( lambda_t *lambda, cons_t *x );
int compile_val( lambda_t *lambda, cons_t *x );
int compile_def( lambda_t *lambda, cons_t *x );
int compile_fun( lambda_t *lambda, cons_t *x );

int compile_args( lambda_t *lambda, value_t args );
int compile_sequence( lambda_t *lambda, value_t x );

lambda_t *compile_closure( symbol_t *name, namespc_t *parent_ns, cons_t *formals, value_t x );

/* API */
lambda_t *compile( value_t x )
{
  /* toplevel compile */
  lambda_t *out = compile_closure(Toplevel, NULL, NULL, x);

  return out;
}

lambda_t *compile_file( const char *fname )
{
  /* compile a rascal file and return the resulting module */
  rl_stream_ref infile = fopen(fname, "rt");

  if ( infile == NULL )
    {
      panic("Couldn't open '%s': reason: %s", fname, strerror(errno));
      return NULL;
    }

  lambda_t *module; symbol_t *module_name; int status;

  /* save here since we'll need to loop */
  save_objects(1, (object_t**)&module);

  module_name = as_symbol(symbol(fname));
  module = make_lambda(module_name, NULL, NULL);

  while ( !feof(infile) )
    {
      value_t expression = read(infile);

      if ( panicking() )
	{
	  module = NULL;
	  break;
	}

      if ( (status=compile_expression(module, expression)) )
	{
	  module = NULL;
	  break;
	}

      emit_lambda_instr(module, op_pop);
    }

  if ( module != NULL )
    {
      finalize_lambda_const(module);
      finalize_lambda_instr(module);
    }

  return module;
}

/* local helper implementations */
lambda_t *compile_closure( symbol_t *name, namespc_t *parent_ns, cons_t *formals, value_t x )
{
  lambda_t *out;

  /* many reallocations occur during compilation, so preserve these at top level */
  save_values(1, &x);
  save_objects(4,
	       (object_t**)&out,
	       (object_t**)&name,
	       (object_t**)&parent_ns,
	       (object_t**)&formals );

  out       = make_lambda(name, parent_ns, formals);

  if ( parent_ns == NULL )
    {
      if ( compile_expression(out, x) < 0 )
	out = NULL;
    }

  else
    {
      if ( compile_sequence(out, x) < 0 )
	out = NULL;
    }

  if ( out != NULL )
    {
      finalize_lambda_const(out);
      finalize_lambda_instr(out);
    }

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
  if ( x == NUL )
    return emit_lambda_instr(lambda, op_load_nul);
  
  size_t loc = put_lambda_const(lambda, x);

  return emit_lambda_instr(lambda, op_load_const, loc);
}

int compile_variable( lambda_t *lambda, value_t x )
{
  int offset;

  int location = lookup_in_lambda_namespc(lambda, as_symbol(x), &offset);

  if ( location < 0 )
    return panic("Undefined reference to '%s'", get_symbol_name(as_symbol(x)));
  
  return emit_lambda_instr(lambda, op_load_variable, offset, location);
}

int compile_combination( lambda_t *lambda, cons_t *form )
{
  if ( is_quote_form(form) )
    return compile_quote(lambda, form);

  if ( is_if_form(form) )
    return compile_if(lambda, form);

  if ( is_do_form(form) )
    return compile_do(lambda, form);

  if ( is_val_form(form) )
    return compile_val(lambda, form);

  if ( is_def_form(form) )
    return compile_def(lambda, form);

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

int compile_val( lambda_t *lambda, cons_t *form )
{
  int status = check_val_syntax(form), offset, location;

  if ( status < 0 )
    return status;

  value_t name = val_name(form, status);
  value_t bind = val_bind(form, status);
  
  location = lookup_in_lambda_namespc(lambda, as_symbol(name), &offset);

  if ( location < 0 )
    return location;

  if ( (status=compile_expression(lambda, bind)) )
    return status;

  return emit_lambda_instr(lambda, op_store_variable, offset, location);
}

int compile_def( lambda_t *lambda, cons_t *form )
{
  int status = check_def_syntax(form);

  if ( status < 0 )
    return status;

  value_t name = def_name(form, status);
  value_t bind = def_bind(form, status);

  int location = define_in_lambda_namespc(lambda, as_symbol(name));

  if ( (status=compile_expression(lambda, bind)) < 0 )
    return status;

  return emit_lambda_instr(lambda, op_store_variable, 0, location);
}

int compile_fun( lambda_t *lambda, cons_t *form )
{
  int status = check_fun_syntax(form), location = -1;

  if ( status < 0 )
    return status;

  value_t name = fun_name(form, status);
  value_t args = fun_args(form, status);
  value_t body = fun_body(form, status);

  if ( name != Fun ) // name supplied, function definition
    location = define_in_lambda_namespc(lambda, as_symbol(name));

  lambda_t *inner_lambda = compile_closure(as_symbol(name), lambda->namespc, as_cons(args), body);

  if ( inner_lambda == NULL )
    return -1;

  int lambda_location = put_lambda_const(lambda, tag_object(inner_lambda));

  emit_lambda_instr(lambda, op_load_const, lambda_location);
  emit_lambda_instr(lambda, op_make_closure);

  if ( location > -1 )
    emit_lambda_instr(lambda, op_store_variable, 0, location);

  return get_instr_len(lambda);
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
