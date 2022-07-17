#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "memutils.h"
#include "numutils.h"
#include "hashing.h"

#include "runtime.h"

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


mk_array_realloc(string, char, true)
mk_array_realloc(vector, value_t, false)
mk_array_realloc(code, short, false)

value_t reallocate( value_t x, size_t n ) { // resize an array's available space
  assert( arrayp( x ) );
  
  if ( stringp( x ) )
    return reallocate_string( x, n );
  
  else if ( vectorp( x ) )
    return reallocate_vector( x, n );
  
  else
    return reallocate_code( x, n );
}

static node_t *forward_node( node_t *x ) {
  if (x == NULL || x->flags == val_fptr)
    return x;

  node_t *new = allocate( sizeof(node_t) );
  memcpy( new, x, sizeof(node_t) );
  x->flags = val_fptr;
  x->key   = tagp( new, tag_table );
  x->left  = new;

  new->left   = forward_node( new->left );
  new->right  = forward_node( new->right );

  return new;
}

static void trace_node( node_t **x ) {
  if (!(*x))
    return;

  if ((*x)->flags == val_fptr) {
    *x = (*x)->left;
    return;
  }

  (*x)->key  = relocate( (*x)->key );
  (*x)->bind = relocate( (*x)->bind );

  trace_node( &(*x)->left );
  trace_node( &(*x)->right );
}

mk_static_forward(pair)
mk_static_forward(closure)

static void trace_array( value_t *a, size_t n );
     
static void trace_closure( value_t cl ) {
  trace_array( pval(cl), 4 );
}

mk_array_forward(string, char, true)
mk_array_forward(vector, value_t, false)
mk_array_forward(code, short, false)

static value_t forward_symbol( value_t x ) {
  void *obspc = allocate( sizeof(symbol_t) );
  memcpy( obspc, pval( x ), sizeof(symbol_t) );
  value_t out = tagp( obspc, tag_symbol );

  char *namespc = allocate( strlen( sname(x) ) + 1 );
  strcpy( namespc, sname( x ) );
  sname( x ) = namespc;
  car( x ) = val_fptr;
  cdr( x ) = out;

  return out;
}

static void trace_symbol( value_t x ) {
  sbind( x ) = relocate( sbind( x ) );
}

static value_t forward_cons( value_t x ) {
  value_t first = tagp( Heap + HUsed, tag_cons),
         *prev = &first;

  do {
    if (car(x) == val_fptr)
      break;
    
     x    = forward_pair(x);
    *prev = x;
     prev = &cdr( x );
     x    = cdr( x );
  } while (consp(x));

  return first;
}

static void trace_cons( value_t x ) {
  value_t *prev = &x;

  do {
    if (car(x) == val_fptr) {
      *prev = cdr( x );
      break;
    }

    car( x ) = relocate( car( x ) );
    prev = &cdr( x );
    x = cdr( x );

  } while (consp(x));
}

static value_t forward_table( value_t x ) {
  node_t *xn = pval( x );
  xn = forward_node( xn );
  return tagp( xn, tag_table );
}

static void trace_table( value_t x ) {
  node_t *n = pval( x );
  trace_node( &n );
}

static value_t forward_array( value_t x ) {
  if ( stringp( x ) )
    return forward_string( x );

  if ( vectorp( x ) )
    return forward_vector( x );

  if ( codep( x ) )
    return forward_code( x );

  __builtin_unreachable();
}


value_t forward( value_t x ) {
  int t = tag( x );
  assert( is_managed( x ) );

  if ( t == tag_cons )
    return forward_cons( x );

  if ( t == tag_symbol )
    return forward_symbol( x );
  
  if ( t == tag_function )
    return forward_closure( x );

  if ( t == tag_table )
    return forward_table( x );

  if ( t == tag_array )
    return forward_array( x );

  __builtin_unreachable();
}


void trace( value_t x ) {
  assert( is_managed( x ) );

  if ( consp( x ) )
    trace_cons( x );

  else if ( symbolp( x ) )
    trace_symbol( x );

  else if ( tablep( x ) )
    trace_table( x );

  else if ( functionp( x ) )
    trace_closure( x );

  else if ( atype( x ) == type_vector )
    trace_array( adata( x ), alen( x ) );

  else
    return;
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

value_t relocate( value_t x ) {
  if ( tag( x ) < tag_cons )
    return x;

  if ( !is_managed( x ) )
    return x;

  if ( car( x ) == val_fptr )
    return cdr( x );

  x = forward( x );
  trace( x );

  return x;
}

// constructors ---------------------------------------------------------------
static value_t intern( symbols_t **table, char *name ) {
  (void)table;
  (void)name;
  return val_nil;
}

pair_t  *mk_pair( void ) { return allocate( sizeof(pair_t) ); }
cons_t  *mk_cons( size_t n ) { return allocate( sizeof(cons_t) * n ); }
node_t  *mk_node( void ) { return allocate( sizeof(node_t) ); }
table_t *mk_table( size_t n ) { return allocate( sizeof(table_t) * n ); }



array_constructor(vector, value_t, false)
array_constructor(string, char, true)
array_constructor(code, short, false)


value_t vector( size_t n, ...) {
  va_list va;
  va_start(va,n);

  index_t base = pushn_s( "vector", n );

  for (size_t i=0; i<n; i++) {
    value_t arg = va_arg(va,value_t);
    Stack[base+i] = arg;
  }

  va_end(va);

  vector_s( n, Stack+base );
  return pop();
}
  
value_t string( char *s ) {
  size_t n = strlen( s );
  char buf[n+1];
  strcpy( buf, s );

  string_s( n, buf );
  return pop();
}

value_t code( size_t n ) {
  code_s( n );

  return pop();
}

value_t cons( value_t ca, value_t cd ) {
  push_s( "cons", cd );
  push_s( "cons", ca );

  cons_s( &Stack[Sp-1], &Stack[Sp-2] );

  return pop();
}

index_t cons_s( value_t *ca, value_t *cd ) {
  cons_t *out    = mk_pair();
  value_t tagged = tagp( out, tag_cons );

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

index_t vector_s( size_t n, value_t *vals ) {
  vector_t *new = mk_vector( n );
  value_t tagged = tagp( new, tag_array );

  if (vals != NULL)
    memcpy( adata( new ), vals, n * sizeof(value_t) );

  
  if ( vals == Stack+Sp-n ) {
    vals[0] = tagged;
    popn( n-1 );
  }

  else
    push_s( "vector", tagged );

  return Sp;
}

index_t string_s( size_t n, char *chars ) {
  string_t *new = mk_string( n );
  strncpy( new->data, chars, n );
  value_t tagged = tagp( new, tag_array );
  push_s( "string", tagged );
  
  return Sp;
}

index_t code_s ( size_t n ) {
  code_t *new    = mk_code( n );
  value_t tagged = tagp( new, tag_array );
  push_s( "code", tagged );
  return Sp;
}

value_t symbol( char *name ) {
  return intern( &Symbols, name );
}

value_t fixnum( long x ) { return tagl( x, tag_fixnum ); }

// utilities ------------------------------------------------------------------
type_t r_type( value_t x ) {
  switch (tag(x)) {
  case tag_immediate: return x>>24;
  case tag_function: return is_managed( x ) ? type_closure : type_builtin;
  case tag_array: return atype( x );
  default: return tag(x);
  }
}

mk_tag_p(fixnum)
mk_tag_p(immediate)
mk_tag_p(cons)
mk_tag_p(symbol)
mk_tag_p(table)
mk_tag_p(function)
mk_tag_p(port)
mk_tag_p(array)
mk_type_p(vector)
mk_type_p(string)
mk_type_p(code)
mk_type_p(closure)
mk_type_p(builtin)

mk_val_p(nil)
mk_type_p(character)
mk_val_p(true)
mk_val_p(false)

bool boolp( value_t x ) { return truep(x) || falsep(x); }

static int order_lists( value_t x, value_t y ) {
  int o;

  while (consp(x) && consp(y)) {
    if ((o = r_order(car(x), car(y))))
      return o;

    x = cdr( x );
    y = cdr( y );
  }
  
  o = 0 -nilp( x ) + nilp( y );

  return o ? : r_order( x, y );
}

static int order_vectors( value_t x, value_t y ) {
  size_t max_cmp = min( alen( x ), alen( y ) );

  value_t *xv = adata( x ), *yv = adata( y );

  int o;
  
  for (size_t i=0; i< max_cmp; i++) {
    if ((o=r_order( xv[i], yv[i])))
      return o;
  }

  return 0 - (alen(x) < alen(y)) + (alen(y) < alen(x));
}

static int order_fixnums( value_t x, value_t y ) {
  return ord_ulong( ival(x), ival(y) );
}

static int order_characters( value_t x, value_t y ) {
  return ord_uint( cval(x), cval(y) );
}

static int order_bools( value_t x, value_t y ) {
  return 0 - (x == val_false) + (y == val_false);
}

static int order_ports( value_t x, value_t y ) {
  return ord_int( fileno( pval( x ) ), fileno( pval( y ) )  );
}

static int order_builtins( value_t x, value_t y ) {
  return ord_ulong( x & ~(value_t)7, y & ~(value_t)7 );
}

static int order_symbols( value_t x, value_t y ) {
  return strcmp( sname( x ), sname( y ) );
}

static int order_strings( value_t x, value_t y ) {
  return strcmp( sdata( x ), sdata( y ) );
}

static int order_bytecodes( value_t x, value_t y ) {
  size_t max_cmp = min( alen( x ), alen( y ) );
  return memcmp( adata( x ), adata( y ), max_cmp * 2 );
}

static int order_tables( value_t x, value_t y ) {
  int order_nodes( node_t *nx, node_t *ny ) {
    int o;
    if (nx == NULL)
      return -1 + (ny == NULL);

    if (ny == NULL)
      return 1;

    if ((o=order_nodes( nx->left, ny->left )))
      return o;

    if ((o=r_order(nx->key, ny->key)))
      return o;

    return order_nodes( nx->right, ny->right );
  }

  return order_nodes( pval(x), pval(y) );
}

static int order_closures( value_t x, value_t y ) {
  int o;

  if ((o=order_symbols(clname(x), clname(y))))
    return o;

  if ((o=r_order(clenvt(x), clenvt(y))))
    return o;

  if ((o=r_order(clvals(x), clvals(y))))
    return o;

  return r_order(clcode(x), clcode(y));
}

int r_order( value_t x, value_t y ) {
  static int (*dispatch[type_pad])( value_t x, value_t y ) = {
    [type_fixnum]  = order_fixnums,   [type_cons]      = order_lists,
    [type_symbol]  = order_symbols,   [ type_table]    = order_tables,
    [type_port]    = order_ports,     [type_character] = order_characters,
    [type_bool]    = order_bools,     [type_nil]       = order_lists,
    [type_vector]  = order_vectors,   [type_string]    = order_strings,
    [type_code]    = order_bytecodes, [type_builtin]   = order_builtins,
    [type_closure] = order_closures
  };

  if (x == y)
    return 0;
  
  type_t xt = r_type( x ), yt = r_type( y );

  if ( xt != yt )
    return ord_int( xt, yt );

  return dispatch[xt]( x, y );
}

hash_t hash_word( value_t x ) {
  return int64hash( x );
}

hash_t hash_symbol( value_t x ) {
  return shash( x );
}

hash_t hash_string( value_t x ) {
  return strhash( sdata( x ) );
}

hash_t hash_code( value_t x ) {
  return memhash( adata( x ), alen( x ) * sizeof(short) );
}

hash_t hash_cons( value_t x ) {
  hash_t h = 0;

  while (consp(x)) {
    hash_t hc = r_hash( car(x) );
    h         = mixhash( h, hc );
  }

  return h;
}

hash_t hash_vector( value_t x ) {
  value_t *vals = adata( x );
  size_t n = alen( x );
  hash_t h = 0;
  
  for (size_t i=0; i<n; i++) {
    hash_t h0 = r_hash( vals[i] );
    h = mixhash( h, h0 );
  }

  return h;
}

hash_t hash_closure( value_t x ) {
  hash_t h  = 0;

  h = mixhash( h, r_hash(clname(x)));
  h = mixhash( h, r_hash(clenvt(x)));
  h = mixhash( h, r_hash(clvals(x)));
  h = mixhash( h, r_hash(clcode(x)));

  return h;
}

hash_t hash_table( value_t x ) {
  hash_t hash_node( node_t *x ) {
    hash_t h = 0;

    h = mixhash(h, r_hash(x->key));
    h = mixhash(h, r_hash(x->bind));

    if (x->left)
      h = mixhash(h, hash_node(x->left));

    if (x->right)
      h = mixhash(h, hash_node(x->right));

    return h;
  }

  return hash_node( pval( x ) );
}

hash_t r_hash( value_t x ) {
  static hash_t (*dispatch[type_pad])( value_t x ) = {
    [type_fixnum]  = hash_word,   [type_cons]      = hash_cons,
    [type_symbol]  = hash_symbol, [type_table]     = hash_table,
    [type_port]    = hash_word,   [type_character] = hash_word,
    [type_bool]    = hash_word,   [type_nil]       = hash_word,
    [type_builtin] = hash_word,   [type_closure]   = hash_closure,
    [type_vector]  = hash_vector, [type_string]    = hash_string,
    [type_code]    = hash_code
  };

  return dispatch[r_type(x)]( x );
}

// safecasts ------------------------------------------------------------------
mk_safe_cast(cons, cons_t*, pval)
mk_safe_cast(symbol, symbol_t*, pval)
mk_safe_cast(fixnum, fixnum_t, ival)
mk_safe_cast(bool, boolean_t, ival)
mk_safe_cast(builtin, builtin_t, pval)
mk_safe_cast(closure, closure_t*, pval)
mk_safe_cast(string, string_t*, pval)
mk_safe_cast(vector, vector_t*, pval)
mk_safe_cast(code, code_t*, pval)
mk_safe_cast(table, table_t*, pval)

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

// error handlign -------------------------------------------------------------
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
