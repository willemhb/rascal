#include "compile.h"
#include "rlvm.h"


/* internal helpers */
void compile_expression( lambda_t *lambda, value_t expression );
void compile_literal( lambda_t *lambda, value_t literal );
void compile_name( lambda_t *lambda, value_t  name );
void compile_funapp( lambda_t *lambda, value_t form );

/* toplevel compile */
lambda_t *compile( value_t expression )
{
  lambda_t *script = make_lambda();

  init_lambda(script, Vm.toplevel->function->namespace);
  compile_expression(script, expression);

  return script;
}
