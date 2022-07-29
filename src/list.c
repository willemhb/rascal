#include <assert.h>

#include "hashing.h"

#include "list.h"

#include "runtime.h"
#include "object.h"
#include "io.h"


#include "function.h"
#include "number.h"

static void init_ncons( cons_t *spc, size_t n, value_t *ini, bool nil_tail ) {
  size_t i;

  value_t last;

  if (nil_tail)
    last = val_nil;

  else
    last = ini[--n];

  for (i=0; i<n; i++) {
    spc[i].car = ini[i];

    if (i+1 == n)
      spc[i].cdr = last;

    else
      spc[i].cdr = tagp(&spc[i+1].car, tag_pair );
  }
}

value_t cons( value_t ca, value_t cd ) {
  push_s( "cons", cd );
  push_s( "cons", ca );

  cons_s( &Stack[Sp-1], &Stack[Sp-2] );

  return pop();
}

value_t list( size_t n, value_t *args ) {
  list_s( n, args );
  return pop();
}

value_t consn( size_t n, value_t *args ) {
  assert( n >= 2 );

  consn_s( n, args );
  return pop();
}

value_t list1( value_t x ) {
  return cons( x, val_nil );
}

value_t list2( value_t x, value_t y ) {
  push_s( "list", x );
  y = list1( y );
  x = pop();
  return cons( x, y );
}

index_t list_s( size_t n, value_t *args ) {
  if (n == 0)
    push_s( "list", val_nil );

  else {
    cons_t *spc = allocate_array( n, sizeof(cons_t) );
    init_ncons( spc, n, args, true );

    if (args+n == &Stack[Sp])
      popn( n );

    push( tagp(spc, tag_pair ) );
  }

  return Sp;
}

index_t consn_s( size_t n, value_t *args ) {
  vargc( "cons*", n, 2 );

  cons_t *spc = allocate_array( n-1, sizeof(cons_t) );
  init_ncons( spc, n, args, false );

  if (args+n == &Stack[Sp])
    popn( n );

  push( tagp( spc, tag_pair ) );

  return Sp;
}

index_t cons_s( value_t *ca, value_t *cd ) {
  cons_t *out    = allocate_pairs(1);
  value_t tagged = tagp( out, tag_pair );

  out->car = *ca;
  out->cdr = *cd;

  if (ca == Stack+Sp && cd == Stack+Sp-1) {
    pop();
    *cd = tagged;

  } else {
    push_s( "cons", tagged );
  }

  return Sp;
}

// accessors & utilities ------------------------------------------------------
value_t cons_nth( value_t c, size_t n ) {
  while(n--)
    c = cdr(c);

  return car(c);
}

value_t cons_xth( value_t c, size_t n, value_t x ) {
  while (n--)
    c = cdr(c);

  car(c) = x;
  return x;
}

value_t cons_nth_s( const char *fname, value_t c, long n ) {
  require( fname,
	   n > 0,
	   "%dl out of bounds for list()",
	   n );

  while (n--)
    c = tocons(fname, c)->cdr;

  return tocons(fname, c)->car;
}

value_t cons_xth_s( const char *fname, value_t c, long n, value_t x ) {
  require( fname,
	   n > 0,
	   "%dl out of bounds for list()",
	   n );

  while (n--)
    c = tocons(fname, c)->cdr;

  tocons(fname, c)->car = x;
  return x;
}

size_t  list_length( value_t x ) {
  size_t n = 0;

  while (is_cons(x)) {
    n++;
    x = cdr(x);
  }

  return n;
}

// predicates -----------------------------------------------------------------
bool is_cons( value_t x ) { return tag(x) == tag_pair; }
bool is_list( value_t x ) { return is_nil(x) || is_cons(x); }

mk_val_p(nil)

// safecasts ------------------------------------------------------------------
mk_safe_cast(cons, cons_t*, pval, true)
     
// methods --------------------------------------------------------------------
int list_order( value_t x, value_t y ) {
  int o;

  while (is_cons(x) && is_cons(y)) {
    if ((o=r_order(car(x), car(y))))
      return o;
    
    x = cdr(x);
    y = cdr(y);
  }
  
  o = 0 - is_nil(x) + is_nil(y);
  
  return o ? : r_order( x, y );
}

size_t list_prin( FILE *ios, value_t c ) {
  size_t out = fprintf( ios, "(" );
  
  while (is_cons(c)) {
    out += r_prin(ios, car(c));
    c    = cdr(c);

    if (is_cons(c))
      out += fprintf( ios, " " );
  }

  if (!is_nil(c)) {
    out += fprintf( ios, " . ");
    out += r_prin( ios, c );
  }

  return out + fprintf( ios, ")" );
}

hash_t list_hash(value_t x) {
  hash_t out = 0;

  value_t buf = x;

  for_cons(&buf, x)
    out = mixhash( out, r_hash(x) );

  return mixhash(out, buf);
}

size_t list_sizeof( value_t x ) {
  if (is_nil(x))
    return 0;

  return sizeof(cons_t);
}

// builtins -------------------------------------------------------------------
void r_builtin(cons) {
  argc( "cons", n, 2 );
  cons_s( &Stack[Sp-2], &Stack[Sp-1] );
}

void r_builtin(consn) {
  consn_s( n, &Stack[Sp-n] );
}

void r_builtin(list) {
  list_s( n, &Stack[Sp-n] );
}

r_predicate(cons)
r_predicate(list)
r_predicate(nil)
r_getter(car, cons)
r_getter(cdr, cons)
r_setter(car, xar, cons)
r_setter(cdr, xdr, cons)

// initialization -------------------------------------------------------------
void list_init( void ) {
  builtin( "cons", builtin_cons );
  builtin( "cons*", builtin_consn );
  builtin( "list", builtin_list );
  builtin( "car", builtin_car );
  builtin( "cdr", builtin_cdr );
  builtin( "xar", builtin_xar );
  builtin( "xdr", builtin_xdr );
  builtin( "cons?", builtin_is_cons );
  builtin( "list?", builtin_is_list );
  builtin( "nil?", builtin_is_nil );
}
