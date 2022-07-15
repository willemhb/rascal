#include <string.h>

#include "numutils.h"
#include "memutils.h"
#include "memory.h"
#include "globals.h"
#include "object.h"

// local utilities -
void manage( void );
void manage_start( void );
void manage_end( void );

void trace_array( value_t *a, size_t n );
void trace_symbols( symbols_t *x );

bool movedp( value_t x ) {
  if (forwardp(car( x )))
    return withinp( ptrval( car(x) ), Heap, HUsed );

  return false;
}

bool managedp( value_t x ) {
  return !( withinp( ptrval( x ), Heap, HSize ) ||
	    withinp( ptrval( x ), Reserve, RSize ) );
}

bool overflowp( int n ) {
  return HUsed + n >= HSize;
}

inline int pow2resize( int new ) { // common dynamic array resize algorithm
  return nextipow2( new );
}

void trace_array( value_t *arr, size_t n ) {
  for (size_t i=0; i<n; i++) {
    value_t tmp = arr[i];
    tmp = relocate( tmp );
    arr[i] = tmp;
  }
}

void trace_symbols( symbols_t *x ) {
  if ( x ) {
    value_t tmp = sbind( x );
    tmp = relocate( tmp );
    sbind( x ) = tmp;

    trace_symbols( x->left );
    trace_symbols( x->right );
  }
}

value_t relocate( value_t x ) {
  if ( !managedp( x ) )
    return x;

  if ( movedp( car( x ) ) )
    return movtag( car( x ), x );

  x = forward( x );

  if ( symp( x ) ) {
    value_t b = sbind( x );
    sbind( x ) = relocate( b );
    
  } else if ( consp( x ) ) {
    value_t cd = cdr( x );
    value_t ca = car( x );
    cdr( x ) = relocate( cd );
    car( x ) = relocate( ca );
  }

  else if ( vecp( x ) )
    trace_array( adata( x ), alen( x ) );
  
  else if ( funp( x ) )
    trace_array( adata( x ), 5 );

  return x;
}

void manage_start( void ) {
  Collecting = true;

  if (Grow) {
    RSize *= Growf;
    Reserve  = realloc_s(Reserve, RSize );
    Grow     = false;
    Grew     = true;
  } else if (Grew) {
    RSize = HSize;
    Heap     = realloc_s(Reserve, RSize );
    Grow = Grew = false;
  }

  // swap spaces
  Heap    = Reserve;
  Reserve = Free;
  Free    = Heap;

  // swap sizes
  RUsed       = RSize;
  RSize       = HeapSize;
  HeapSize    = RUsed;
  RUsed       = HeapUsed;
  HeapUsed    = 0;
}

void manage_end( void ) {
  Collecting = false;
  Grow       = HeapUsed >= Resizef * HeapSize;
  
}

void manage(void) {
  manage_start();

  trace_symbols( Symbols );
  trace_array( Stack, Sp );

  manage_end();
}

void *allocbytes( size_t n ) {
  size_t padded = aligned( n, sizeof( cons_t ) );

  void *out = Heap + HUsed;
  HUsed += padded;

  memset( out, 0, padded );
  return out;
}

void *allocate( size_t n ) { 
  if ( !!Collecting && overflowp( n ) )
    manage();

  return allocbytes( n );
}


value_t forward( value_t x ) {
  size_t n = r_size( x );
  void *dst = allocate( n );
  void *src = arrayp( x ) ? adata( x ) : ptrval( x );
  memcpy( dst, src, n );

  car( x ) = settag( dst, tag_forward );
  return movtag( dst, x );
}

void *reallocate( value_t x, size_t n ) {
  assert( arrayp( x ) );

  size_t o = arr_resize( n );  
}
