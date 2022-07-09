#include "vm/compile.h"
#include "describe/sequence.h"
#include "runtime/object.h"
#include "runtime/error.h"

static int compile_quote( value_t form, value_t *vals, value_t *code, value_t *envt );
static int compile_if( value_t form, value_t *vals, value_t *code, value_t *envt );
static int compile_macro( value_t form, value_t *vals,  value_t *code, value_t *envt );
static int compile_lambda( value_t form, value_t *vals, value_t *code, value_t *envt );
static int compile_do( value_t form, value_t *vals, value_t *code, value_t *envt );
static int compile_funcall( value_t form, value_t *vals, value_t *code, value_t *envt );
static int compile_literal( value_t form, value_t *vals, value_t *code, value_t *envt );
static int compile_reference( value_t form, value_t *vals, value_t *code, value_t *envt );

static int compile_quote( value_t c, value_t *vals, value_t *code, value_t *envt ) {
  (void)envt;

  require( !nullp( cdr( c ) ),
	   "bad syntax in quote: not enough inputs" );
  require( nullp( cdr( cdr( c ) ) ),
	   "bad syntax in quote: too many inputs" );

  
}

static int add_constant( value_t c, value_t *vals ) {
  int i;
  value_t x;
  
  for_vec(vals, i, x) {
    
  }
  
}
