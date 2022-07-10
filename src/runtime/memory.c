
#include <string.h>

#include "memutils.h"
#include "numutils.h"

#include "describe/sequence.h"
#include "runtime/object.h"
#include "runtime/memory.h"
#include "runtime/stack.h"

inline int pow2resize( int new ) { // common dynamic array resize algorithm
  return nextipow2( new );
}

inline int arr_resize( int new ) { // the more conservative Python algorithm
  return (new + (new >> 3) + 6) & ~(size_t)3;
}

void *getword( void *p ) {              // get the correctly aligned allocated word p is in
  return ((void*)(((value_t)p)&~7ul));
}

int getoff( void *p ) { return p - getword( p ); }

inline bool overflowp(int n) {
  return Free + n >= Heap + HeapSize;
}

void *allocate(flags_t fl, int n ) {
  if (flagp( fl, memfl_global ) )
    return malloc_s( n );

  int padded = aligned( n, 16 );

  if (overflowp(padded)) manage();

  void  *out  = Free;

  Free        += padded;
  HeapUsed    += padded;

  memset( out, 0, padded );
  return out;
}

object_t *reallocate( object_t *ob, int o, int n ) {
  void *newspc, *oldspc;

  if ( Collecting ) {
    int base = TypeSizes[ob->type];
    
    // allocate
    newspc = Free;
    Free  += aligned( base + n, 16 );

    // copy, save object data
    memcpy( newspc, ob, base );
    oldspc = ob->data;

    // set forward pointer
    car( ob ) = rfptr;
    cdr( ob ) = tagp( newspc, ob->type&7 );

    // point object data at correct memory, copy old data
    ob = newspc;

    if ( o == n && n == 0 )
      return ob;
    
    newspc = asuptr( ob ) + o;

  } else {
    save( tagp( ob, ob->type & 7 ) );
    newspc = allocate( n, globalp( ob ) );
    restore( (value_t*)&ob );
    ob = asptr( ob );
    oldspc = ob->data;
  }

  memcpy( newspc, oldspc, min( o, n ) );
  ob->data = newspc;
  ob->size = n;
  return ob;
}

// GC -------------------------------------------------------------------------
static value_t move_cons( value_t x ) {
  uchar** buf = &Free;
  value_t out = (value_t)Free;

  while ( consp( x ) ) {
    void *spc  = *buf;
    *buf      += sizeof(cons_t);
    memcpy( asptr( x ), spc, sizeof( cons_t ) );

    value_t tmp = cdr( x );

    if ( consp( tmp ) )
      cdr( spc ) = tagp( *buf, tag_cons );

    car( x ) = rfptr;
    cdr( x ) = tagp( spc, tag_cons );
 
    x = tmp;
  }

  x = out;
  
  while ( consp( x ) ) {
    value_t tmp = car( x );
    tmp = relocate( tmp );
    car( x ) = tmp;

    if ( !consp( cdr( x ) ) ) {
      tmp = cdr( x );
      tmp = relocate( tmp );
      cdr( x ) = tmp;
    }

    x = cdr( x );
  }

  return out;
}

static void trace_array( value_t *arr, int n ) {
  for (int i=0; i<n; i++) {
    value_t tmp = arr[i];
    tmp = relocate( tmp );
    arr[i] = tmp;
  }
}

static void trace_table( root_t *x ) {
  void trace_nodes( node_t *x ) {
    if ( x ) {
      x->bind = relocate( x->bind );
      trace_nodes( x->left );
      trace_nodes( x->right );
    }
  }
  trace_nodes( x->data );
}

static value_t move_tup( tuple_t *x ) {
  x = (tuple_t*)reallocate( (object_t*)x, x->size, x->size );
  trace_array( x->data, x->len );
  return tagp( x, x->type & 7 );
}

static value_t move_table( root_t *x ) {
  node_t *move_node( node_t *x ) {
    if ( x ) {
      x = (node_t*)reallocate( (object_t*)x, x->base.size, x->base.size );
      x->left  = move_node( x->left );
      x->right = move_node( x->right );
    }
    return x;
  }

  x = (root_t*)reallocate( (object_t*)x, 0, 0 );
  node_t *tmp = x->data;
  tmp = move_node( tmp );
  x->data = tmp;
  return tagp( x, x->type & 7 );
}

static value_t move_closure( closure_t *x ) {
  x = (closure_t*)reallocate( asptr( x ), 0, 0 );
  trace_array( (value_t*)x, 4 );
  return tagp( x, tag_closure );
}

value_t relocate( value_t x ) {
  if ( immediatep( x ) ) return x;
  if ( globalp( asptr( x ) ) ) return x;
  if ( movedp( x ) ) return cdr( x );

  type_t t = val_typeof( x );

  switch ( t&7 ) {
  case tag_cons: return move_cons( x );
  case tag_tuple: return move_tup( asptr( x ) );
  case tag_table: return move_table( asptr( x ) );
  case tag_closure: return move_closure( asptr( x ) );
  default: break;
  }

  // for no particular reason all other object types end up having exactly one field
  // that needs to be traced at offset 2 from the object head
  object_t *o = asptr( x );
  o = reallocate( o, o->size, o->size );
  x = obslots( o )[2];
  x = relocate( x );
  obslots( o )[2] = x;

  return tagp( o, t&7 );
}

// toplevel helpers for GC ----------------------------------------------------
static void managestart( void ) {
  Collecting = true;

  if (Grow) {
    RSize *= Growf;
    Reserve  = realloc_s(Reserve, RSize );
    Grow     = false;
    Grew     = true;
  } else if (Grew) {
    RSize = HeapSize;
    Heap     = realloc_s(Reserve, RSize );
    Grow = Grew = false;
  }

  // swap spaces
  Free    = Heap;
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

static void manageend( void ) {
  Collecting = false;
  Grow       = HeapUsed >= Resizef * HeapSize;
}

void manage(void) {
  managestart();

  trace_table( &Symbols );
  trace_table( &Syntax );

  trace_array(Stack, Sp);
  trace_array(Dump, Dp);

  Error = relocate( Error );
  Ins   = relocate( Ins );
  Outs  = relocate( Outs );

  manageend();
}
