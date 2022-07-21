#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>

#include "memutils.h"
#include "numutils.h"
#include "hashing.h"

#include "runtime.h"
#include "object.h"

#include "symbol.h"
#include "list.h"
#include "function.h"
#include "array.h"
#include "number.h"

// memory management ----------------------------------------------------------
size_t calc_array_size( size_t n_bytes ) { // Python array allocation algorithm
  return (n_bytes + (n_bytes >> 3) + 6) & ~(size_t)3;
}

bool check_heap_overflow( size_t n_bytes ) {
  return HUsed + n_bytes > HSize;
}

void *allocate( size_t n_bytes ) {
  size_t padded = aligned( n_bytes, sizeof(cons_t) );
  if ( !Collecting && check_heap_overflow( n_bytes ) )
    collect_garbage();
  
  void *out  = Heap + HUsed;
  HUsed     += padded;

  memset( out, 0, padded );
  return out;
}


static void trace_symbols( symbols_t *x ) {
  if ( x ) {
    x->base.bind = relocate( x->base.bind );
    trace_symbols( x->left );
    trace_symbols( x->right );
  }
}

static void trace_array( value_t *base, size_t n ) {
  for (size_t i=n; i>0; i--) {
    value_t tmp = base[i-1];
    base[i-1]     = relocate( tmp );
  }
}

int collect_garbage( void ) {
  Collecting = true;
  
  if ( Grow ) {
    HSize *= Growf;
    Reserve = realloc_s(Reserve, HSize );
    Grow = false;
    Grew = true;
  } else if ( Grew ) {
    Reserve = realloc_s(Reserve, HSize );
    Grow = Grew = false;
  }

  // swap spaces
  uchar *Tmp = Heap;
  Heap       = Reserve;
  Reserve    = Tmp;
  RUsed      = HUsed;
  HUsed      = 0;

  // trace roots
  trace_symbols( Symbols );
  trace_array( Stack, Sp );

  // update state
  Collecting = false;
  Grow       = HUsed >= HSize * Resizef;

  return 0;
}

value_t forward( value_t x ) {
  size_t n = is_cons(x) ? sizeof(cons_t) : ob_base_size(x);

  void *spc = allocate( n );

  memcpy( spc, pval( x ), n );

  if (ob_is_array(x)) {
    size_t elsize  = Ctype_size( ob_Ctype(x) );
    void *dspc     = allocate( ( asize( x ) + !!ob_encoding(x)) * elsize );
    memcpy( dspc, adata( x ), alength( x ) * elsize );
    adata( spc ) = dspc;
  }

  value_t out = tagp( spc, tag(x));

  car( x ) = val_forward;
  cdr( x ) = out;

  return out;
}

value_t relocate( value_t x ) {
  if ( tag( x ) < tag_pair )
    return x;

  if ( !is_managed( x ) )
    return x;

  if ( car( x ) == val_forward )
    return cdr( x );

  x = forward( x );

  if ( is_cons( x ) ) {
    value_t ca = car( x );
    value_t cd = cdr( x );

    cdr( x ) = relocate( cd );
    car( x ) = relocate( ca );
  } else if ( is_symbol( x ) ) {
    value_t bind = assymbol(x)->bind;
    assymbol(x)->bind = relocate( bind );
    
  } else if ( is_closure( x ) ) {
    trace_array( pval(x) + sizeof(value_t), 3 );
    
  } else if ( is_vector( x ) ) {
    trace_array( adata(x), alength(x) );
    
  }

  return x;
}

// stack manipulation ---------------------------------------------------------
index_t push( value_t x ) {
  Stack[Sp++] = x;
  return Sp-1;
}

index_t pushn( size_t n ) {
  Sp += n;
  return Sp-n;
}

index_t push_s( const char *fname, value_t x ) {
  require( fname, Sp < N_STACK, "stack overflow" );

  return push( x );
}

index_t pushn_s( const char *fname, size_t n ) {
  require( fname, Sp + n < N_STACK, "stack overflow" );
  return pushn( n );
}

value_t pop( void ) { return Stack[--Sp]; }
value_t popn( size_t n ) { return Stack[(Sp=Sp-n)+n-1]; }

value_t pop_s( const char *fname ) {
  require( fname, Sp > 0, "stack underflow" );
  return pop();
}

value_t popn_s( const char *fname, size_t n ) {
  require( fname, Sp >= n, "stack underflow" );
  return popn( n );
}

// error handling -------------------------------------------------------------
static void perror_v( const char *fname, const char *fmt, va_list va) {
  fprintf( stderr, "%s: error: ", fname );
  vfprintf( stderr, fmt, va );
  fprintf( stderr, ".\n" );
}

void error(const char *fname, const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  perror_v( fname, fmt, va );
  va_end(va);
  longjmp( Toplevel, 1 );
}

void require(const char *fname, bool test, const char *fmt, ...) {
  if ( test )
    return;

  va_list va;
  va_start(va, fmt);
  perror_v( fname, fmt, va );
  va_end(va);
  longjmp( Toplevel, 1 );
}

// builtins -------------------------------------------------------------------
void r_builtin(errorb) {
  
}

// initialization -------------------------------------------------------------
void runtime_init( void ) {
  // create heaps
  Heap    = malloc_s( HSize );
  Reserve = malloc_s( HSize );

  // initialize module globals
  r_main  = symbol("&main");
  r_args  = symbol("&args");
  r_kw_ok = symbol(":okay");

  // initialize builtins
  builtin( "error!", builtin_errorb );
}
