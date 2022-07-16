#include <string.h>

#include "error.h"
#include "stack.h"
#include "memory.h"
#include "object.h"

#define Arg1    (Stack[Sp-n])
#define Arg2    (Stack[Sp-n+1])
#define Arg3    (Stack[Sp-n+2])

#define Arg(i)  (Stack[Sp-n+i])

// cons builtins --------------------------------------------------------------
void r_aligned r_cons( size_t n ) {
  argc( "cons", n, 2 );
  cons_s( &Arg1, &Arg2 );
}

void r_aligned r_ncat( size_t n ) {
  vargc( "xat", n, 2 );
  for (size_t i=1; i<n; i++) { // do n-1 times for n lists
    require( "xat",
	     listp( Stk1st ) && listp( Stk2nd ),
	     "# wanted 2 list()'s" );
    
     Stk2nd = ncat( Stk2nd, Stk1st );
     Sp--;
  }
}

void r_aligned r_nrev( size_t n ) {
  argc( "nrev", n, 1 );
  require( "nrev", listp( Tos ), "# wanted a list()" );
  Tos = nrev( Tos );
}

r_predicate(cons)
r_predicate(list)
r_getter(car, cons)
r_getter(cdr, cons)
r_setter(car, xar, cons)
r_setter(cdr, xdr, cons)

void r_aligned r_vector( size_t n ) { vector_s( n, &Tos-n ); }

r_predicate(vector)

void r_aligned r_nth( size_t n ) {
  argc( "nth", n, 2 );
  aref_s( "nth", &Arg1, &Arg2 );
}

void r_aligned r_xth( size_t n ) {
    argc( "xth", n, 3 );
    axef_s( "xth", &Arg1, &Arg2, &Arg3 );
}

void r_aligned r_put( size_t n ) {
  argc( "put", n, 2 );
  aput_s( "put", &Arg1, &Arg2 );
}

void r_aligned r_len( size_t n ) {
  argc( "len", n, 1 );

  type_t t = oargt( "len", Tos, 5,
		    type_cons,
		    type_nil,
		    type_string,
		    type_vector,
		    type_bytecode );
  
  if ( t == type_nil )
    n = 0;

  else if ( t == type_cons )
    n = clength( Tos );

  else
    n = length( Tos );

  Tos = mk_fixnum( n );
}

// strings --------------------------------------------------------------------
void r_aligned r_string( size_t n ) {
  
}

// symbols --------------------------------------------------------------------
r_predicate(symbol)
r_predicate(gensym)
r_predicate(keyword)
r_predicate(bound)

// arithmetic -----------------------------------------------------------------
static inline fixnum_t uargsub( fixnum_t x ) { return -x; }
static inline fixnum_t uargadd( fixnum_t x ) { return +x; }
static inline fixnum_t uargdiv( fixnum_t x ) { return 1/x; }
static inline fixnum_t uargmul( fixnum_t x ) { return x; }
static inline fixnum_t uargmod( fixnum_t x ) { return x; }

r_arithmetic(add, "+", +, false, 1, uargadd, -1)
r_arithmetic(sub, "-", -, false, 1, uargsub, -1)
r_arithmetic(mul, "*",*, false, 2, uargmul,  0)
r_arithmetic(div, "/", /, true, 2, uargdiv, 0)
r_arithmetic(mod, "mod", %, true, 2, uargmod, 0)
r_arithmeticp(eqp, "=", ==)
r_arithmeticp(ltp, "<", <)

// virtual machine ------------------------------------------------------------

void r_aligned r_compile( size_t n ) {
  dup();                     // save original expression
  vector_s( 8, NULL );       // constant pool
  bytecode_s( 128, NULL );   // code sequence
  push( val_nil );           // environment

  
}

void r_aligned r_apply( size_t n );


value_t mk_builtin( char *name, builtin_t callback ) {
  value_t out    = symbol( name );
  sbind( out )   = settag( callback, tag_function );
  sflags( out ) |= symfl_const;
  
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
  
  mk_builtin( "vector", r_vector );
  mk_builtin( "vector?", r_vectorp );
  mk_builtin( "nth", r_nth );
  mk_builtin( "xth", r_xth );
  mk_builtin( "put", r_put );
  mk_builtin( "len", r_len );

  mk_builtin( "string", r_string );
  mk_builtin( "string?", r_stringp );

  mk_builtin( "bytecode", r_bytecode );
  mk_builtin( "bytecode?", r_bytecodep );

  mk_builtin( "symbol?", r_symbolp );
  mk_builtin( "gensym?", r_gensymp );
  mk_builtin( "keyword?", r_keywordp );
  mk_builtin( "bound?", r_boundp );

  // arithmetic
  mk_builtin( "+", r_add );
  mk_builtin( "-", r_sub );
  mk_builtin( "*", r_mul );
  mk_builtin( "/", r_div );
  mk_builtin( "mod", r_mod );
  mk_builtin( "=", r_eqp );
  mk_builtin( "<", r_ltp );
}
