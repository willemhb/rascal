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

uchar_t *findmap(void_t *ptr) {
  size_t offset;
  uchar_t *map, *uptr = ptr;
  
  if (withinp( uptr, Heap, NHeap ) ) {
    offset = uptr - Heap;
    map    = HeapMap;
  }

  else {
    offset = uptr - Reserve;
    map    = ReserveMap;
  }

  return map+offset;
}

inline bool_t overflowp(size_t n) {
  return NHeap + n >= HeapUsed;
}

bool_t globalp(void_t *ptr) {
  if (Collecting)
    return !withinp(ptr, Reserve, NReserve );
  return withinp(ptr, Heap, NHeap );
}

inline uchar_t getmemfl(void_t *ptr) {
  return *findmap(ptr);
}

uchar_t  setmemfl(void_t *ptr, uchar_t fl) {
  uchar_t *map = findmap(ptr);
  return (*map |= fl);
}

uchar_t clearmemfl(void_t *ptr, uchar_t fl) {
  uchar_t *map = findmap(ptr);
  return (*map &= ~fl);
}

void_t *allocate(size_t nbytes, bool_t global) {
  if (global) return malloc_s(nbytes);
  
  size_t padded = aligned( nbytes, 8);

  if (overflowp(padded)) manage();

  void_t *out  = Free;
  Free        += padded;
  memset( out, 0, padded );

  return out;
}

object_t *construct(type_t type, size_t extra, bool_t global) {
  size_t total   = TypeSizes[type] + extra;
  object_t *out  = allocate( total, global );
  obtype( out )  = type;
  obsize( out )  = total;
  return out;
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

  // swap spaces
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

  object_t *ob = (object_t*)Reserve, *end = (object_t*)(Reserve+ReserveSize);
  uchar_t *map = ReserveMap;

  while ( ob < end ) {
    while ( *map & mem_raw ) {
      (*map++) = 0;
      *(value_t*)ob = rnull;
      ob++;

      if (ob >= end)
	goto finalize_end;
    }

    
    size_t size   = aligned( obsize(ob), 8);
    size_t nwords = size / 8;
    
    if ( obtag( ob ) == tag_moved )
      obhead( ob ) = obslot( ob, 1 ) = rnull;
    
    else if (obfinalizep(ob))
      Finalize[obtype(ob)](ob);
    
    memset( ob, 0, size );
    memset( map, 0, nwords );

    ob  += nwords;
    map += nwords;
  }

 finalize_end:
  // clear the trace flag on anything that was moved
  for (size_t i=0; i<Sp; i++) untrace(Stack[i]);
  for (size_t i=0; i<Dp; i++) untrace(Dump[i]);

  // clear the traversal flag for any globals
  untrace( (value_t)Symbols );
  untrace( (value_t)Globals );
  untrace( Function );
  untrace( (value_t)Error );
  untrace( (value_t)Ins );
  untrace( (value_t)Outs );
  untrace( (value_t)Errs );
}

void untrace( value_t x ) {
  if (immediatep(x))
    return;

  if (obtracep(x)) {
    obtracep(x) = false;

    type_t xt = obtype( x );

    if ( Untrace[xt] )
      Untrace[xt]( x );
  }
}

value_t trace( value_t val ) {
  if (immediatep( val )) return val;
  if (movedp( val )) return car( val );
  if ( obtracep( val ) ) return val;
  obtracep( val ) = true;

  if (!globalp(ptr(val)))
    relocate( &val, &Free, &MapFree, &HeapUsed );

  value_t (*trace_dispatch)(value_t v) = Trace[obtype(val)];

  return trace_dispatch ? trace_dispatch( val ) : val;
}

static void tracearray(value_t *vals, size_t limit) {
  for (size_t i=0; i<limit; i++) {
    value_t val = vals[i];
    val         = trace( val );
    vals[i]     = val;
  }
}

void manage(void) {
  managestart();

  trace((value_t)Globals);
  trace((value_t)Symbols);
  tracearray(Stack, Sp);
  tracearray(Dump, Dp);

  // trace other roots
  Function = trace( Function );
  Error    = (object_t*)trace((value_t)Error);
  Ins      = (object_t*)trace((value_t)Ins);
  Outs     = (object_t*)trace((value_t)Outs);
  Errs     = (object_t*)trace((value_t)Errs);

  manageend();
}

void relocate( value_t *buf, uchar_t **space, uchar_t **map, size_t *used ) {
  type_t xt = obtype( *buf );

  if (Relocate[xt])
    Relocate[xt](buf, space, map, used );

  else {
    uchar_t *oldmap = findmap(ptr(*buf)), *newmap = *map;
    uchar_t *oldspace = ptr(*buf), *newspace = *space;
 
    size_t obsz   = obsize(*buf);
    size_t padded = aligned( obsz, 8 );
    size_t nwords = padded / 8;

    *space += padded;
    *used += padded;
    *map  += nwords;

    memcpy( newspace, oldspace, obsz );
    memcpy( newmap, oldmap, nwords );
    memset( oldmap, 0, nwords );

    obslot(oldspace, 0) &= ~1ul;
    obslot(oldspace, 1) = (value_t)newspace;
  }
}

// initialization -------------------------------------------------------------
void initmemory(void) {
  // initialize counters and arities
  Symcnt = Sp = Bp = Fp = Dp = 0;

  NHeap = NReserve = N_STACK*8;                       // number of words in heap
  HeapSize = ReserveSize = NHeap * sizeof(value_t);
  HeapUsed = ReserveUsed = 0;

  // allocate heap and map space
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
