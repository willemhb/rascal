#include "rascal.h"

#include "vm/apply.h"


// local declarations ---------------------------------------------------------
value_t apply_builtin( builtin_t fun, value_t *args, size_t nargs );
value_t apply_form( form_t form, value_t *args, size_t nargs );
value_t apply_closure( closure_t *cl, size_t nargs );

// exports --------------------------------------------------------------------
value_t rascal_apply( value_t fun, value_t args ) {
  (void)fun;
  (void)args;
  return rnull;
}
