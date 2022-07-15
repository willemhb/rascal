#include "error.h"
#include "stack.h"


void r_cons( int n ) {
  argc( "cons", n, 2 );
  cons_t *out = allocate( sizeof(cons_t) );
  cdr( out ) = pop();
  car( out ) = Tos;
  Tos        = settag( out, tag_cons );
}

void r_car( int n ) {
  argc( "car", n, 1);
  require( "car",
	   consp( Tos ),
	   "# expected a cons(), got something else" );
  Tos = car( Tos );
}

void r_cdr( int n ) {
  argc( "cdr", n, 1);
  require( "cdr",
	   consp( Tos ),
	   "# expected a cons(), got something else" );
  Tos = car( Tos );
}

void r_xar( int n ) {
  argc( "xar", n, 2 );
  require( "xar",
	   consp( Peek(2) ),
	   "# expected a cons(), got something else" );

  rotate();
  car( Tos ) = Peek( 2 );
  pop();
}

void r_xdr( int n ) {
  argc( "xdr", n, 2 );
  require( "xdr",
	   consp( Peek(2) ),
	   "# expected a cons(), got something else" );
  rotate();
  cdr( Tos ) = Peek( 2 );
  Sp--;
}

void r_vec( int n ) {
  int arity = arr_resize( n );
  vector_t new = allocate( (arity+1) * sizeof(value_t));

  new[0] = settag( n, tag_fixnum );
  memcpy( new+1, Stack+Sp-n, n * sizeof(value_t) );
  popn( n );
  push( settag( new, tag_vector ) );
}

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

value_t mk_builtin( char *name, builtin_t callback ) {
  value_t out  = symbol( name );
  sbind( out ) = settag( callback, tag_function );
  return out;
}

void mk_builtins( void ) {
  // constructors
  mk_builtin( "cons", r_cons );
  mk_builtin( "cons?", r_consp );
  mk_builtin( "car", r_car );
  mk_builtin( "cdr", r_cdr );
  mk_builtin( "xar", r_xar );
  mk_builtin( "xdr", r_xdr );
  
  mk_builtin( "vec", r_vec );
  mk_builtin( "nth", r_nth );
  mk_builtin( "len", r_len );
}
