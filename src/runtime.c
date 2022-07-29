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

  size_t nwords = padded / 8;
  size_t Hwords = HUsed / 8;
  
  void *out  = Heap + HUsed;
  void *omap = Map  + Hwords;

  HUsed     += padded;

  memset( out, 0, padded );
  memset( omap, 0, nwords );
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
    Map     = realloc_s(Map, HSize / 8 );
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

size_t argc(const char *fname, size_t got, size_t expect ) {
  require(fname,
	  got == expect,
	  "# wanted %zu arguments, got %zu",
	  expect,
	  got );

  return got;
}

size_t vargc(const char *fname, size_t got, size_t expect ) {
    require(fname,
	  got >= expect,
	  "# wanted at least %zu arguments, got %zu",
	  expect,
	  got );

  return got;
}

size_t oargc(const char *fname, size_t got, size_t n, ...) {
  static const char *ofmt = "%zu, ";
  static const char *lfmt = "or %zu arguments, ";
  
  assert(n >= 2);
  va_list va;
  va_start(va, n);
  
  if (n == 2) {
    const char *fmt = "# wanted %zu or %zu arguments, got %zu";
    size_t n1 = va_arg(va, size_t);
    size_t n2 = va_arg(va, size_t);
    va_end(va);
    
    require( fname,
	     got == n1 || got == n2,
	     fmt,
	     n1,
	     n2,
	     got );

    return got;
  }

  FILE *tmp = tmpfile();

  assert(tmp);
  
  size_t bufsize = 1 + fprintf( tmp, "# wanted " );
  bool test = false;

  for (size_t i=0;i<n;i++) {
    const char *fmt = i+1 == n ? lfmt : ofmt;
    size_t o        = va_arg(va, size_t);
    bufsize        += fprintf( tmp, fmt, o );
    test            = test || got == o;
  }

  va_end(va);
  bufsize += fprintf( tmp, "got %zu", got );

  char buf[bufsize];

  fseek( tmp, 0, SEEK_SET );
  fgets( buf, bufsize, tmp );
  fclose( tmp );

  require(fname,
	  test,
	  buf );

  return got;
}

type_t argt(const char *fname, value_t got, type_t expect) {
  type_t out = r_type(got);
  
  require( fname,
	   out == expect,
	   "# wanted a %s(), got a %s()",
	   Typenames[expect],
	   Typenames[out] );
  
  return out;
}

type_t oargt(const char *fname, value_t got, size_t n, ...) {
  static const char *ofmt = "a %s(), ";
  static const char *lfmt = "or a %s(), ";
  
  assert(n >= 2);
  va_list va;
  va_start(va, n);

  type_t tg = r_type(got);
  
  if (n == 2) {
    const char *fmt = "# wanted a %s() or %s(), got a %s()";
    type_t t1 = va_arg(va, type_t);
    type_t t2 = va_arg(va, type_t);
    va_end(va);
    
    require( fname,
	     tg == t1 || tg == t2,
	     fmt,
	     Typenames[t1],
	     Typenames[t2],
	     Typenames[tg] );

    return tg;
  }

  FILE *tmp = tmpfile();

  assert(tmp);

  size_t bufsize = 1;
  bufsize += fprintf( tmp, "# wanted " );
  bool test = false;

  for (size_t i=0;i<n;i++) {
    const char *fmt = i+1 == n ? lfmt : ofmt;

    type_t o = va_arg(va, type_t);
    bufsize += fprintf(tmp, fmt, Typenames[o] );
    test     = test || tg == o;
  }

  va_end(va);
  bufsize += fprintf( tmp, "got a %s()", Typenames[tg] );

  char buf[bufsize];

  fseek( tmp, 0, SEEK_SET );
  fgets( buf, bufsize, tmp );
  fclose( tmp );

  require(fname,
	  test,
	  buf );
  
  return got;
}

size_t s_argc( const char *fname, value_t form, size_t expect ) {
  require( fname, is_cons(form), "error: syntax: not a list" );
  size_t length = list_length(form);
  require(fname,
	  length == expect,
	  "error: syntax: not expected %zu expressions, got %zu",
	  expect,
	  length );
  return length;
}

size_t s_vargc( const char *fname, value_t form, size_t expect ) {
  require( fname, is_cons(form), "error: syntax: not a list" );
  size_t length = list_length(form);
  require(fname,
	  length >= expect,
	  "error: syntax: not expected at least %zu expressions, got %zu",
	  expect,
	  length );
  return length;
}

// builtins -------------------------------------------------------------------
void r_builtin(getenv) {
  argc("sys/getenv", n, 1 );
  type_t t = oargt("sys/getenv", n, 2, type_string, type_symbol );
  
  char *key;

  if (t == type_symbol)
    key = sname(Tos);

  else
    key = tostring("sys/getenv", Tos)->data;

  char *value = getenv(key);

  if (value == NULL)
    Tos = val_nil;

  else
    Tos = string( value );
}

void r_builtin(exit) {
  oargc("sys/exit", n, 2, 0, 1 );

  int status = 0;

  if (n == 1) {
    value_t arg = pop();
    status = tofixnum( "sys/exit", arg );
  }

  exit(status);
}

void r_builtin(system) {
  argc("sys/system", n, 1 );

  char *command = tostring("sys/system", Tos )->data;
  int result = system(command);
  Tos = fixnum(result);
}

void r_builtin(errorb) {
  argc( "error!", n, 1 );
  argt( "error!", Tos, type_string );
  value_t arg = pop();

  char *msg = is_empty(arg) ? "" : adata(arg);

  error( "exec", msg );
}

// initialization -------------------------------------------------------------
void runtime_init( void ) {
  // create heaps
  Heap    = malloc_s( HSize );
  Map     = malloc_s( HSize / 8 );
  Reserve = malloc_s( HSize );

  // initialize module globals
  r_main  = symbol("&main");
  r_argc  = symbol("&argc");
  r_args  = symbol("&args");
  r_kw_ok = symbol(":okay");

  // initialize builtins
  builtin( "sys/exit", builtin_exit );
  builtin( "sys/getenv", builtin_getenv );
  builtin( "sys/system", builtin_system );
  builtin( "error!", builtin_errorb );
}
