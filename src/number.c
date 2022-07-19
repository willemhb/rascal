#include <string.h>
#include <assert.h>
#include <stdlib.h>


#include "numutils.h"

#include "number.h"
#include "object.h"
#include "symbol.h"
#include "array.h"
#include "function.h"
#include "runtime.h"

// constructors ---------------------------------------------------------------
value_t fixnum( long x )   { return tagi( x, tag_fixnum ); }
value_t boolean( int x )   { return x ? val_true : val_nil; }
value_t character( int c ) { return tagi(c, tag_character); }

// utilities ------------------------------------------------------------------
bool fixnum_fits( fixnum_t x, Ctype_t ctype ) {
  return x >= Ctype_min_int[ctype] && x <= Ctype_max_int[ctype];
}

size_t fixnum_init( const char *fname, value_t x, Ctype_t c, void *spc ) {
  fixnum_t f = tofixnum( fname, x );

  require( fname,
	   fixnum_fits(f, c),
	   "%ld not between %ld and %ld",
	   f,
	   Ctype_min_int[c],
	   Ctype_max_int[c] );

  memcpy( spc, &f, Ctype_size( c ) );
  
  return Ctype_size( c );
}

// builtins ------------------------------------------------------------------
void builtin_fixnum( size_t n ) {
  argc( "fixnum", n, 1);

  type_t arg1t = oargt( "fixnum", Tos, 3, type_fixnum, type_character, type_string );

  if (arg1t == type_fixnum)
    return;

  else if (arg1t == type_character)
    Tos = fixnum( cval( Tos ) );

  else {
    require( "fixnum",
	     Tos != val_estr,
	     "parse error: \"\" is not a number" );
    char *sbuf;
    long l = strtol( s8data( Tos ), &sbuf, 0 );
    require( "fixnum",
	     *sbuf == '\0',
	     "parse error: \"%s\" is not a number",
	     s8data(Tos) );
    Tos = fixnum(l);
  }
}

void builtin_character( size_t n ) {
  argc( "character", n, 1 );
  type_t arg1t = oargt( "character", Tos, 2, type_fixnum, type_character );

  if (arg1t == type_character)
    return;

  Tos = character( ival(Tos) );
}

void builtin_boolean( size_t n ) {
  argc( "bool", n, 1);
  Tos = boolean( Cbool( Tos ) );
}

		      
void builtin_add( size_t n ) {
  vargc( "+", 1, n );

  fixnum_t acc = tofixnum( "+", Stack[Sp-n] );

  for (size_t i=1; i < n; i++)
    acc += tofixnum("+", Stack[Sp-n+i] );

  popn(n);
  push( fixnum( acc ) );
}

void builtin_sub( size_t n  ) {
  vargc( "-", 1, n );

  fixnum_t acc = tofixnum( "-", Stack[Sp-n] );

  if (n == 1)
    Tos = fixnum( -acc );

  else
    for (size_t i=1; i < n; i++)
      acc -= tofixnum("-", Stack[Sp-n+i] );

  popn(n);
  push( fixnum( acc ) );
}

void builtin_mul( size_t n ) {
  vargc( "*", 2, n );
  
  fixnum_t acc = tofixnum( "*", Stack[Sp-n] );

  for (size_t i=1; i < n && acc != 0; i++)
    acc *= tofixnum("*", Stack[Sp-n+i] );

  popn(n);
  push( fixnum( acc ) );
}

void builtin_div( size_t n ) {
  vargc( "/", 1, n );
  
  fixnum_t acc = tofixnum( "/", Stack[Sp-n] );

  if (n == 1)
    push( fixnum( 1/acc ) );

  else
    for (size_t i=1; i < n && acc != 0; i++) {
      require( "/",
	       Stack[Sp-n+i] != 0,
	       "zero-division" );
      acc /= tofixnum("/", Stack[Sp-n+i] );
    }

  popn(n);
  push( fixnum( acc ) );
}

void builtin_mod( size_t n ) {
  argc( "mod", 2, n );

  fixnum_t q = tofixnum("mod", Stack[Sp-2]);
  fixnum_t d = tofixnum("mod", Stack[Sp-1]);

  require( "mod",
	   d != 0,
	   "zero-division" );

  popn(2);
  push( fixnum( q%d ) );
}

void r_builtin(is_eqn) {
  vargc( "=", n, 2 );
  fixnum_t acc = tofixnum("=", Stack[Sp-n] );
  bool sentinel = true;
  for (size_t i=1; sentinel && i<n; i++) {
    fixnum_t tmp = tofixnum("=", Stack[Sp-n+i]);
    sentinel = acc == tmp;
    acc = tmp;
  }

  popn( n );
  push( boolean( sentinel ) );
}

void r_builtin(is_ltn) {
  vargc( "<", n, 2);

  fixnum_t acc = tofixnum("<", Stack[Sp-n] );
  bool sentinel = true;
  for (size_t i=1; sentinel && i<n; i++) {
    fixnum_t tmp = tofixnum("<", Stack[Sp-n+i]);
    sentinel = acc < tmp;
    acc = tmp;
  }

  popn( n );
  push( boolean( sentinel ) );
}

r_predicate(fixnum)
r_predicate(character)
r_predicate(boolean)
r_predicate(false)     
r_predicate(true)

// predicates -----------------------------------------------------------------
mk_tag_p(fixnum)
mk_val_p(true)
mk_val_p(false)
mk_type_p(character)

bool is_boolean( value_t x ) { return is_true(x) || is_false(x); }

// safecasts ------------------------------------------------------------------
mk_safe_cast(character, character_t, cval, false)
mk_safe_cast(fixnum, fixnum_t, ival, false)
mk_safe_cast(boolean, boolean_t, cval, false)

// initialization -------------------------------------------------------------
void number_init( void ) {
  builtin( "fixnum", builtin_fixnum );
  builtin( "character", builtin_character );
  builtin( "bool", builtin_boolean );

  builtin( "fixnum?", builtin_is_fixnum );
  builtin( "character?", builtin_is_character );
  builtin( "bool?", builtin_is_boolean );
  builtin( "true?", builtin_is_true );
  builtin( "false?", builtin_is_false );
  
  builtin( "+", builtin_add );
  builtin( "-", builtin_sub );
  builtin( "*", builtin_mul );
  builtin( "/", builtin_div );
  builtin( "mod", builtin_mod );
  builtin( "=", builtin_is_eqn );
  builtin( "<", builtin_is_ltn );
}
