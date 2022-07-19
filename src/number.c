#include <string.h>
#include <assert.h>

#include "numutils.h"

#include "number.h"
#include "object.h"
#include "runtime.h"

// constructors ---------------------------------------------------------------
value_t fixnum( long x )   { return tagi( x, tag_fixnum ); }
value_t boolean( int x )   { return x ? val_true : val_nil; }
value_t character( int c ) { return tagi(c, tag_character); }

// utilities ------------------------------------------------------------------
bool fixnum_fits( fixnum_t x, Ctype_t ctype ) {
  return x >= Ctype_min_int[ctype] && x <= Ctype_max_int[ctype];
}

void fixnum_init( const char *fname, value_t x, Ctype_t c, void *spc ) {
  fixnum_t f = tofixnum( fname, x );

  require( fname,
	   fixnum_fits(f, c),
	   "%ld not between %ld and %ld",
	   f,
	   Ctype_min_int[c],
	   Ctype_max_int[c] );

  memcpy( spc, &f, Ctype_size( c ) );
}

// builtins ------------------------------------------------------------------
void builtin_fixnum( size_t n );
void builtin_character( size_t n );
void builtin_boolean( size_t n );

static value_t u_noop( fixnum_t x ) { return fixnum(x); }
static value_t u_sub( fixnum_t x ) { return fixnum(-x); }
static value_t u_div( fixnum_t x ) { return fixnum(1/x); }

r_arithmetic(add, "+", +, false, 1, u_noop, -1)
r_arithmetic(sub, "-", -, false, 1, u_sub, -1)
r_arithmetic(mul, "*", *, false, 2, u_noop, 0)
r_arithmetic(div, "/", /, true, 1, u_div, 0)
r_arithmetic(mod, "mod", %, true, 2, u_noop, 0)

r_arithmetic_p(is_eqn, "=", == )
r_arithmetic_p(is_ltn, "<", < )

r_predicate(fixnum)
void builtin_is_character( size_t n );
void builtin_is_boolean( size_t n );
void builtin_is_true( size_t n );
void builtin_is_false( size_t n );

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
