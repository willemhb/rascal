#include "runtime/memory.h"
#include "memutils.h"
#include "numutils.h"
#include "runtime/stack.h"
#include "object/pair.h"
#include <string.h>

// static forward declarations
// exports --------------------------------------------------------------------

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



void *getmap(void *p) {
  size_t offset;
  void *wp = getword( p );
  
  uchar_t *map, *up = wp;

  if (withinp( up, Heap, NHeap ) ) {
    offset = up - Heap;
    map    = HeapMap;
  } else if ( withinp( up, Reserve, NReserve ) ) {
    offset = up - Reserve;
    map    = ReserveMap;
  } else {
    map    = up;
    offset = 7;
  }

  return map+offset;
}

inline bool_t overflowp(size_t n) {
  return NHeap + n >= HeapUsed;
}

inline uchar_t getmemfl(void_t *p) {
  return *(uchar_t*)(getmap(p));
}

uchar_t  setmemfl(void_t *p, uchar_t fl) {
  uchar_t *map = getmap(p), out = *map;
  *map |= fl;
  return out;
}

uchar_t clearmemfl(void_t *p, uchar_t fl) {
  uchar_t *map = getmap(p), out = *map;
  *map &= ~fl;

  return out;
}

void_t *allocate(flags_t fl, size_t n ) {
  if (flagp( fl, memfl_global ) )
    return malloc_s( n );

  size_t padded = aligned( n, 8 );
  size_t nwords = padded / 8;
  
  if (overflowp(padded)) manage();

  void_t  *out  = Free;
  uchar_t *map  = MapFree;
  
  Free        += padded;
  MapFree     += nwords;
  HeapUsed    += nwords;

  memset( out, 0, padded );
  memset( map, (fl&255)|mem_allocated, nwords );
  return out;
}

object_t *reallocate( object_t *ob, size_t ol, size_t nl, void **p ) {
  void *op = *p, *np;

  if ( globalp( op ) ) {
    
     np = realloc_s( op, nl );
     *p  = np;
     return ob;
  }
  
  if ( overflowp( nl ) ) {
    saven( 1, ob );

    np = allocate( 0, nl );
    p  = *p;
    op = *p;

    restoren( 1, &ob );
  } else {
    np = allocate( 0, nl );
  }
  
  memcpy( np, op, min( ol, nl ) );
  *p   = np;
  
  return ob;
}

static void managestart(void) {
  Collecting = true;

  if (Grow) {
    NReserve *= Growf;
    Reserve  = realloc_s(Reserve, NReserve );
    Grow     = false;
    Grew     = true;
  } else if (Grew) {
    NReserve = NHeap;
    Heap     = realloc_s(Reserve, NReserve );
    Grow = Grew = false;
  }

  // swap opaces
  Free    = Heap;
  Heap    = Reserve;
  Reserve = Free;
  Free    = Heap;

  // swap sizes
  ReserveUsed = NReserve;
  NReserve    = NHeap;
  NHeap       = ReserveUsed;
  ReserveUsed = HeapUsed;
  HeapUsed    = 0;
}

static void manageend( void ) {
  Collecting = false;
  Grow       = HeapUsed >= Resizef * NHeap;
}


void manage(void) {
  managestart();

  trace_array();
  trace((value_t)Symbols);
  tracearray(Stack, Op);
  tracearray(Dump, Dp);

  // trace other roots
  Function = trace( Function );
  Error    = (object_t*)trace((value_t)Error);
  Ins      = (object_t*)trace((value_t)Ins);
  Outs     = (object_t*)trace((value_t)Outs);
  Errs     = (object_t*)trace((value_t)Errs);

  manageend();
}

void relocate( value_t *buf, uchar_t **opace, uchar_t **map, size_t *used ) {
  type_t xt = obtype( *buf );

  if (Relocate[xt])
    Relocate[xt](buf, opace, map, used );

  else {
    uchar_t *oldmap = findmap(ptr(*buf)), *newmap = *map;
    uchar_t *oldopace = ptr(*buf), *newopace = *opace;
 
    size_t obsz   = obsize(*buf);
    size_t padded = aligned( obsz, 8 );
    size_t nwords = padded / 8;

    *opace += padded;
    *used += padded;
    *map  += nwords;

    memcpy( newopace, oldopace, obsz );
    memcpy( newmap, oldmap, nwords );
    memset( oldmap, 0, nwords );

    obslot(oldopace, 0) &= ~1ul;
    obslot(oldopace, 1) = (value_t)newopace;
  }
}

// initialization -------------------------------------------------------------
void initmemory(void) {
  // initialize counters and arities
  Symcnt = Sp = Bp = Fp = Dp = 0;

  NHeap = NReserve = N_STACK*8;                       // number of words in heap
  HeapSize = ReserveSize = NHeap * sizeof(value_t);
  HeapUsed = ReserveUsed = 0;

  // allocate heap and map nace
  Heap       = malloc_s(NHeap);
  Reserve    = malloc_s(NReserve);
  HeapMap    = malloc_s(NHeap);
  ReserveMap = malloc_s(NHeap);

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
