
#include <string.h>

#include "memutils.h"
#include "numutils.h"

#include "runtime/object.h"
#include "runtime/memory.h"
#include "runtime/stack.h"

/*
  stolen python algorithm for future use.
  static int
  list_resize(PyListObject *self, Py_ssize_t newsize)
{
    PyObject **items;
    size_t new_allocated, num_allocated_bytes;
    Py_ssize_t allocated = self->allocated;

    // Bypass realloc() when a previous overallocation is large enough
    // to accommodate the newsize.  If the newsize falls lower than half
    // the allocated size, then proceed with the realloc() to shrink the list.
    
    if (allocated >= newsize && newsize >= (allocated >> 1)) {
        assert(self->ob_item != NULL || newsize == 0);
        Py_SET_SIZE(self, newsize);
        return 0;
    }

    // This over-allocates proportional to the list size, making room
    // for additional growth.  The over-allocation is mild, but is
    // enough to give linear-time amortized behavior over a long
    // sequence of appends() in the presence of a poorly-performing
    // system realloc().
    // Add padding to make the allocated size multiple of 4.
    // The growth pattern is:  0, 4, 8, 16, 24, 32, 40, 52, 64, 76, ...
    // Note: new_allocated won't overflow because the largest possible value
    //      is PY_SSIZE_T_MAX * (9 / 8) + 6 which always fits in a size_t.
    new_allocated = ((size_t)newsize + (newsize >> 3) + 6) & ~(size_t)3;
    // Do not overallocate if the new size is closer to overalocated size
    // than to the old size.
    
    if (newsize - Py_SIZE(self) > (Py_ssize_t)(new_allocated - newsize))
        new_allocated = ((size_t)newsize + 3) & ~(size_t)3;

    if (newsize == 0)
        new_allocated = 0;
    num_allocated_bytes = new_allocated * sizeof(PyObject *);
    items = (PyObject **)PyMem_Realloc(self->ob_item, num_allocated_bytes);
    if (items == NULL) {
        PyErr_NoMemory();
        return -1;
    }
    self->ob_item = items;
    Py_SET_SIZE(self, newsize);
    self->allocated = new_allocated;
    return 0;
}
 */

inline size_t pow2resize( size_t new ) { // common dynamic array resize algorithm
  return nextipow2( new );
}

inline size_t arr_resize( size_t new ) { // the more conservative Python algorithm
  return (new + (new >> 3) + 6) & ~(size_t)3;
}

void *getword( void *p ) {              // get the correctly aligned allocated word p is in
  return ((void*)(((value_t)p)&~7ul));
}

size_t getoff( void *p ) { return p - getword( p ); }

inline bool overflowp(size_t n) {
  return Free + n >= Heap + HeapSize;
}

void *allocate(flags_t fl, size_t n ) {
  if (flagp( fl, memfl_global ) )
    return malloc_s( n );

  size_t padded = aligned( n, 8 );

  if (overflowp(padded)) manage();

  void  *out  = Free;
  
  
  Free        += padded;
  HeapUsed    += padded;

  memset( out, 0, padded );
  return out;
}

value_t reallocate( value_t ob, size_t o, size_t n ) {
  bool g = globalp( asptr( ob ) );

  save( ob );
  
  void *newspc = allocate( n, g );

  restore( &ob );

  void *oldspc = obdata( ob );

  memcpy( newspc, oldspc, min( o, n ));

  obdata( ob ) = newspc;

  return ob;
}

// GC -------------------------------------------------------------------------
static value_t relocate( value_t x );

static void    trace_array( value_t *a, size_t n );
static void    trace_table( root_t *a );
static void    trace_table_data( node_t *n );
static void    trace_symbol( value_t x );

static value_t  move_symbol( value_t x );
static value_t  move_cons( value_t x );
static value_t  move_port( value_t x );
static value_t  move_closure( value_t x );
static value_t  move_vec( value_t x );
static value_t  move_table( value_t x );
static node_t  *move_table_data( node_t *x, void **spc );
static value_t  move_binary( value_t x );

static value_t relocate( value_t x ) {
  if ( immediatep( x ) )
    return x;

  if ( globalp( asptr( x ) ) )
    return x;

  if ( movedp( x ) )
    return cdr( x );
  
  if ( consp( x ) )
    return move_cons( x );

  if ( symbolp( x ) )
    return move_symbol( x );

  if ( portp( x ) )
    return move_port( x );

  if ( closurep( x ) )
    return move_closure( x );

  if ( vectorp( x ) )
    return move_vec( x );

  if ( binaryp( x ) )
    return move_binary( x );

  if ( tablep( x ) )
    return move_table( x );

  __builtin_unreachable();
}

// implement for array types --------------------------------------------------
static void trace_array( value_t *vals, size_t max ) {
  for (size_t i=0; i<max; i++) {
    value_t val = vals[i];
    val         = relocate( val );
    vals[i]     = val;
  }
}

static value_t move_vec( value_t x ) {  
  static const size_t base = sizeof(vector_t);

  value_t out       = tagp( Free, tag_vector );
  size_t xlen       = vlen( x );
  size_t xpad       = vcap( x );
  
  size_t data_used  = sizeof(value_t) * xlen;
  size_t data_alloc = sizeof(value_t) * xpad;
  size_t total      = base + data_alloc;
  size_t padded     = aligned( total, 16 );

  void  *oldspc     = asptr( x ), *newspc  = Free;
  void  *oldvals    = vdata( x ), *newvals = Free + sizeof(vector_t);
  
  Free             += padded;
  memcpy( newspc, oldspc, base );
  
  vdata( newspc )   = newvals;
  memcpy( newvals, oldvals, data_used );
  
  car( oldspc )     = rfptr;
  cdr( oldspc )     = tagp( newspc, tag_vector );

  trace_array( newvals, xlen );

  return out;
}

// implement for table types --------------------------------------------------
static void trace_table( root_t *x ) {
  trace_table_data( mapdata( x ) );
}

static void trace_table_data( node_t *x ) {
  if ( !x ) return;

  value_t bind = mapbind( x );
  bind         = relocate( bind );
  mapbind( x ) = bind;

  if ( !globalp( x ) ) {
    value_t val = mapval( x );
    val         = relocate( val );
    mapval( x ) = val;
  }

  trace_table_data( mapleft( x ) );
  trace_table_data( mapright( x ) );
}

static value_t move_table( value_t x ) {
  static const size_t rootsz = sizeof(root_t);
  static const size_t nodesz = sizeof(node_t);

  void *oldspc  = asptr( x ), *newspc = Free;
  size_t nbuf = mapcap( x ), total  = rootsz + nodesz * nbuf;

  value_t out   = tagp( Free, tag_table );
  Free         += aligned( total, 16 );

  memcpy( newspc, oldspc, rootsz );

  void *newnodes = (uchar*)newspc + rootsz, *oldnodes = mapdata( out );

  car( x ) = rfptr;
  cdr( x ) = out;

  mapdata( out )  = move_table_data( oldnodes, &newnodes );

  trace_table( newspc );

  return out;
}

static node_t *move_table_data( node_t *n, void **buf ) {
  static const size_t nodesz = sizeof(node_t);

  if ( n ) {
    node_t *out      = *buf, *tmp;
    *((uchar**)buf) += nodesz;

    memcpy( out, n, nodesz );

    car( n ) = rfptr;
    cdr( n ) = (value_t)out;

    n             = out;
    tmp           = mapleft( n );
    tmp           = move_table_data( tmp, buf );
    mapleft( n )  = tmp;
    tmp           = mapright( n );
    tmp           = move_table_data( tmp, buf );
    mapright( n ) = tmp;
  }

  return n;
}

// implement for symbol types -------------------------------------------------
static void trace_symbol( value_t x ) {
  value_t bind = symbind( x );

  bind         = relocate( bind );
  symbind( x ) = bind;
}

static value_t move_symbol( value_t x ) {
  static const size_t sz = sizeof( symbol_t );

  void *oldspc           = asptr( x );
  void *newspc           = Free;
  value_t out            = tagp( newspc, tag_symbol );
  
  size_t symsz           = sz + strlen( symname( x ));
  size_t padded          = aligned( symsz, 16 );
  Free                  += padded;
  
  memcpy( oldspc, newspc, symsz );
  
  car( x )               = rfptr;
  cdr( x )               = out;
  
  trace_symbol( out );

  return out;
}

// implement for pair types ---------------------------------------------------
static value_t move_cons( value_t x ) {
  static const size_t sz = sizeof( cons_t );

  value_t out = tagp( Free, tag_cons );
  value_t cx  = x;

  // compress
  while ( consp( cx ) ) {
    void *spc  = Free;
    Free      += sz;
    
    memcpy( spc, asptr( cx ), sz );

    x          = cdr( cx );
    car( cx )  = rfptr;
    cdr( cx )  = tagp( spc, tag_cons );

    if ( consp( x ) )
      cdr( (uchar*)spc ) = tagp( (uchar*)spc + sizeof(cons_t), tag_cons );
    else
      cdr( spc ) = x;

    cx         = x;
  }
  
  // trace cars
  cx = out;

  while ( consp( cx ) ) {
    x         = car( cx );
    x         = relocate( x );
    car( cx ) = x;
  }

  return out;
}

// implement for other primitive types ----------------------------------------
static value_t move_port( value_t x ) {
  static const size_t base = sizeof(port_t);

  size_t total = base + strlen( portname( x ) );
  void *spc    = Free, *oldspc = asptr( x );
  value_t out  = tagp( spc, tag_port );
  Free        += aligned( total, 16 );

  memcpy( spc, oldspc, total );

  car( x )     = rfptr;
  cdr( x )     = out;

  trace_array( spc, 2 );

  return out;
}

static value_t move_closure( value_t x ) {
  static const size_t base = sizeof(closure_t);

  void *oldspc = asptr( x );
  void *newspc = Free;
  Free        += base;

  memcpy( newspc, oldspc, base );

  value_t out   = tagp( newspc, tag_closure );
  
  car( oldspc ) = rfptr;
  cdr( oldspc ) = out;

  trace_array( newspc, 4 );

  return out;
}

static value_t move_binary( value_t x ) {
  static const size_t base = sizeof(binary_t);

  void *newspc = Free, *oldspc = asptr( x ), *newd = Free + base, *oldd = bdata( x );

  size_t elsize     = Ctype_size( val_eltype( x ) );
  size_t used       = elsize * blen( x );
  size_t allocated  = elsize * bcap( x );
  Free             += aligned( base + allocated, 16 );
  value_t out       = tagp( Free, tag_binary );
  
  memcpy( newspc, oldspc, base );
  memcpy( newd, oldd, used );
  
  car( x )          = rfptr;
  cdr( x )          = out;

  return out;
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

// initialization -------------------------------------------------------------
void initmemory(void) {
  // initialize counters and arities
  Symcnt = Sp = Bp = Fp = Dp = 0;

  HeapSize = RSize = N_STACK*sizeof(cons_t); // number of pairs in heap
  HeapUsed = RUsed = 0;

  // allocate heap and map nace
  Heap       = malloc_s(HeapSize);
  Reserve    = malloc_s(RSize);

  // initialize GC flags
  Grow = Grew = Collecting = false;

  // initialize GC load factors
  Collectf = 1.0;
  Resizef  = 0.685;
  Growf    = 2.0;

  // set Free
  Free    = Heap;

  // clean stacks
  memset(Stack, 0, N_STACK * sizeof(value_t));
  memset(Dump, 0, N_STACK * sizeof(value_t));
}
