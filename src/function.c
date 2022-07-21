
#include "memutils.h"

#include "function.h"

#include "object.h"
#include "symbol.h"
#include "runtime.h"
#include "number.h"

// constructors ---------------------------------------------------------------
value_t builtin( char *name, void (*callback)( size_t n ) ) {
  value_t sname = symbol( name );
  builtin_t *spc = malloc_s( sizeof(builtin_t ) );

  init_builtin( spc, sname, callback );
  return tagp( spc, tag_boxed );
}

value_t closure( value_t envt, value_t vals, value_t code ) {
  push_s( "closure", envt );
  push_s( "closure", vals );
  push_s( "closure", code );

  closure_s( &Sref(3), &Sref(2), &Sref(1) );
  return pop();
}

index_t closure_s( value_t *envt, value_t *vals, value_t *code ) {
  closure_t *new = allocate( sizeof(closure_t) );

  init_closure( new, *envt, *vals, *code );

  if (code == Stack+Sp-1)
    popn(3);

  push( tagp( new, tag_boxed ) );
  return Sp;
}

// predicates -----------------------------------------------------------------
mk_type_p(builtin)
mk_type_p(closure)

bool is_function( value_t x ) { return is_builtin(x) || is_closure(x); }

// safecasts ------------------------------------------------------------------
mk_safe_cast(builtin, builtin_t*, pval, true)
mk_safe_cast(closure, closure_t*, pval, true)

// methods --------------------------------------------------------------------
int closure_order( value_t x, value_t y ) {
  int o;
  
  if ((o=r_order(asclosure(x)->code, asclosure(y)->code)))
      return o;

    if ((o=r_order(asclosure(x)->envt, asclosure(y)->envt)))
      return o;

    return r_order(asclosure(x)->vals, asclosure(y)->vals);
}

// builtins -------------------------------------------------------------------
r_predicate(builtin)
r_predicate(closure)
r_predicate(function)

// initialization -------------------------------------------------------------
void function_init( void ) {
  // create module builtins
  builtin("builtin?", builtin_is_builtin );
  builtin("closure?", builtin_is_closure );
  builtin("function?", builtin_is_function );
}

