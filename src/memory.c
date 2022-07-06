#include "memory.h"
#include "memutils.h"
#include "stack.h"
#include "symbol.h"
#include "table.h"
#include "tuple.h"
#include "rstring.h"
#include "pair.h"
#include "function.h"
#include <string.h>

// static forward declarations
static void tracearray( value_t *vals, size_t limit );
static void tracetable( object_t *ob );

// exports --------------------------------------------------------------------
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

static object_t *relocate( object_t *ob ) {
  size_t used     = obsize(ob);
  size_t padded   = used;
  size_t dataloc  = 0;
  size_t datasize = 0;
  type_t type     = obtype(ob);
  void_t *obdata  = NULL;

  switch ( type ) {
  case type_tuple:
    obdata = tupledata(ob);

    break;

  case type_string:
    obdata   = schars(ob);
    dataloc  = sizeof(value_t) * 3;
    datasize = slen(ob) * sizeof(char_t);
    used   += slen(ob) * sizeof(char_t);
    padded += scap(ob) * sizeof(char_t);
    break;

  default:
    break;
  }

  padded         = aligned(padded, 8);

  object_t *new  = (object_t*)Free;
  Free          += padded;

  memcpy( new, ob, used );
  obhead( ob )   = tagv( new, tag_moved );

  if ( obdata != NULL ) {
    void_t *dataspace = (uchar_t*)new + TypeSizes[type];
    memcpy( dataspace, obdata, datasize );
    ((void_t**)ob)[dataloc] = dataspace;
  }
  
  return new;
}

object_t *construct(type_t type, size_t extra, bool_t global) {
  size_t total   = TypeSizes[type] + extra;
  object_t *out  = allocate( total, global );
  obhead(out)    = (total<<8)|type;
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

static void manageend(void) {
  Collecting = false;
  Grow       = HeapUsed >= Resizef * NHeap;

  // clear the traversal flags
  uchar_t *base = Reserve;

  for (size_t i=0; i<NReserve; i++) {
    clearmemfl(base+i, mem_traversed );
  }

  base = Heap;

  for (size_t i=0; i<NHeap; i++) {
    clearmemfl(base+i, mem_traversed );
  }
}

static object_t *traceobject(object_t *ob);

static value_t tracevalue(value_t xv) {
  if (nullp(xv)) return xv;
  if (immediatep(xv)) return xv;
  if (movedp(xv)) return (value_t)getfp(xv);

  return (value_t)traceobject( ptr(xv) );
}

static object_t *traceobject(object_t *ob) {
  if (nullp(ob)) return ob;
  if (movedp(ob)) return ob;
  if (getmemfl(ob) & mem_traversed) return ob;

  setmemfl( ob, mem_traversed);

  if (!globalp(ob)) {
    ob = relocate( ob );
  }

  type_t ot = obtype( ob );
  value_t tmpv;
  object_t *tmpo;

  switch ( ot ) {
  case type_symbol:
    tmpv = tracevalue( symbind( ob ) );
    symbind( ob ) = tmpv;
    break;

  case type_tuple:
    tracearray(tvals(ob), tlen(ob));
    break;

  case type_table:
    tracetable( ob );
    break;

  case type_pair ... type_cons:
    tmpv = tracevalue( cdr( ob ) );
    cdr( ob ) = tmpv;
    tmpv = tracevalue( car( ob ) );
    car( ob ) = tmpv;
    break;

  case type_function:
    tmpo = traceobject( funenvt( ob ) );
    funenvt( ob ) = tmpo;
    tmpo = traceobject( funformals( ob ) );
    funformals( ob ) = tmpo;
    tmpo = traceobject( funbody( ob ) );
    funbody( ob ) = tmpo;
    break;
    
  default:
    break;
  }
  
  return ob;
}

static void tracetable(object_t *node) {
  if (!node) return;

  value_t entry = tabentry(node);
  
  entry = tracevalue(entry);
  tabentry(node) = entry;

  tracetable(tabright(node));
  tracetable(tableft(node));
}

static void tracearray(value_t *vals, size_t limit) {
  for (size_t i=0; i<limit; i++) {
    value_t val = vals[i];
    val         = tracevalue(val);
    vals[i]     = val;
  }
}

void manage(void) {
  managestart();

  tracearray(tvals(Globals), tlen(Globals));
  tracetable(Symbols);
  tracearray(Stack, Sp);
  tracearray(Dump, Dp);
  
  Function = tracevalue(Function);
  Error    = traceobject(Error);
  Ins      = traceobject(Ins);
  Outs     = traceobject(Outs);
  Errs     = traceobject(Errs);

  manageend();
}

// initialization -------------------------------------------------------------
void initmemory(void) {
  // initialize counters and arities
  Symcnt = Sp = Bp = Fp = Dp = 0;

  NHeap = NReserve = N_HEAP*sizeof(pair_t);

  HeapUsed = ReserveUsed = 0;

  // allocate heap and map space
  Heap       = malloc_s(NHeap);
  Reserve    = malloc_s(NReserve);
  HeapMap    = malloc_s(N_STACK);
  ReserveMap = malloc_s(N_STACK);

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
