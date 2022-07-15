#include <string.h>

#include "error.h"
#include "stack.h"
#include "memory.h"
#include "object.h"

// cons builtins --------------------------------------------------------------
void r_cons( int n ) {
  argc( "cons", n, 2 );
  cons_t *out = allocate( sizeof(cons_t) );
  cdr( out ) = pop();
  car( out ) = Tos;
  Tos        = settag( out, tag_cons );
}

void r_ncat( int n ) {
  vargc( "ncat", n, 2 );

  value_t out = Unbound;

  for (int i=0; i<n; i++) {
    value_t tmp = pop();

    if (out == Unbound)
      out = tmp;

    if ( nilp( tmp ) )
      continue;

    else if (consp( tmp )) {
      while (consp( tmp ) && consp( cdr( tmp )))
	tmp = cdr( tmp );

      cdr( tmp ) = Tos;
      
    } else {
      r_error( "ncat", "# wanted a list()" );
      
    }
  }

  push( out );
}

void r_nrev( int n ) {
  argc( "nrev", n, 1 );
  require( "nrev", listp( Tos ), "# wanted a list()" );

  
}

r_predicate(cons)
r_predicate(list)
r_getter(car, cons, car)
r_getter(cdr, cons, cdr)
r_setter(xar, cons, car)
r_setter(xdr, cons, cdr)

void r_vec( int n ) {
  int arity = arr_resize( n );
  vector_t new = allocate( (arity+1) * sizeof(value_t));

  new[0] = settag( n, tag_fixnum );
  memcpy( new+1, Stack+Sp-n, n * sizeof(value_t) );
  popn( n );
  push( settag( new, tag_vector ) );
}

r_predicate(vec)

void r_nth( int n ) {
  argc( "nth", n, 2 );
  require( "nth",
	   fixnump( Tos ),
	   "# expected int(), got something else" );

  n = longval( pop() );

  require( "nth",
	   vecp( Tos ),
	   "# expected vec(), got something else" );

  if ( n < 0 )
    n += alen( Tos );

  require( "nth",
	   n > 0 && n < (int)alen(Tos),
	   "%d out of bounds for vec() of size %d",
	   n,
	   (int)alen(Tos) );

  Tos = vdata( Tos )[n];
}

void r_len( int n ) {
  argc( "len", n, 1 );

  require( "len",
	   consp( Tos ) || vecp( Tos ),
	   "# wanted collection()" );

  if ( vecp( Tos ) )
    n = alen( Tos );

  else {
    n = 0;
    value_t x;

    for_cons( &Tos, x )
      n++;
  }

  Tos = mk_fixnum( n );
}


value_t mk_builtin( char *name, builtin_t callback ) {
  value_t out  = symbol( name );
  sbind( out ) = settag( callback, tag_function );
  return out;
}

void mk_builtins( void ) {
  // constructors
  mk_builtin( "cons", r_cons );
  mk_builtin( "cons?", r_consp );
  mk_builtin( "list?", r_listp );
  mk_builtin( "car", r_car );
  mk_builtin( "cdr", r_cdr );
  mk_builtin( "xar", r_xar );
  mk_builtin( "xdr", r_xdr );
  
  mk_builtin( "vector", r_vec );
  mk_builtin( "vector?", r_vecp );
  mk_builtin( "nth", r_nth );
  mk_builtin( "xth", r_xth );
  mk_builtin( "put", r_put );
  mk_builtin( "len", r_len );

  mk_builtin( "symbol?", r_symp );
  mk_builtin( "gensym?", r_gensymp );
}
