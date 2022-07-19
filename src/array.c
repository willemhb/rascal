#include <string.h>
#include <assert.h>

#include "numutils.h"

#include "array.h"
#include "runtime.h"
#include "io.h"
#include "object.h"

#include "function.h"
#include "list.h"
#include "number.h"
#include "symbol.h"

// constructors ---------------------------------------------------------------
value_t vector( size_t n, value_t *args ) {
  if ( n == 0 )
    return val_evec;

  if ( !in_stack( args ) ) {
    index_t base = pushn( n );
    value_t *buf = Stack+base;
    memcpy( buf, args, n * sizeof(value_t) );
    vector_s( n, buf );
  }

  else
    vector_s( n, args );
  
  return pop();
}

index_t vector_s( size_t n, value_t *vals ) {
  if ( n == 0 ) {
    push( val_evec );
    return Sp;
  }
  
  size_t a = calc_array_size( n );
  vector_t *new  = allocate_vector( a );

  init_vector( new, n, a );

  value_t tagged = tagp( new, tag_boxed );
  
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


value_t binary( size_t n, Ctype_t ctype, void *args ) {
  if ( n == 0 )
    return empty_bins[ctype];

  if (in_heap(args)) {
    size_t total = n * Ctype_size(ctype);
    uchar buf[total];
    memcpy(buf, args, total);
    binary_s( n, ctype, buf );
  }

  else
    binary_s( n, ctype, args );

  return pop();
}

index_t binary_s ( size_t n, Ctype_t ctype, void *args ) {
  if ( n == 0 ) {
    push( empty_bins[ctype] );
    return Sp;
  }
  size_t s  = calc_array_size( n );
  size_t es = Ctype_size(ctype);
  binary_t *new = allocate_table( sizeof(binary_t), s, es );
  init_binary( new, ctype, n, s );

  if (args != NULL)
    memcpy( adata(new), args, n * es );

  value_t tagged = tagp( new, tag_boxed );

  push( tagged );

  return Sp;
}

value_t string( char *chrs ) {
  size_t n = strlen(chrs);

  if (n == 0)
    return val_estr;

  char buf[n+1];
  strcpy(buf, chrs);
  
  string_s( n, buf );
  return pop();
}

value_t string_s( size_t n, char *chrs ) {
  if (n == 0)
    push(val_estr);

  else {
    size_t s = calc_array_size( n );
    string_t *new = allocate_table( sizeof(string_t), s+1, 1 );
    init_string( new, n, s );

    if (chrs != NULL)
      strcpy( adata(new), chrs );

    push( tagp( new, tag_boxed ) );
  }

  return Sp;
}

// accessors & utilities ------------------------------------------------------
static char *empty_array_str( value_t array ) {
  if (array == val_estr)
    return "\"\"";

  if (array == val_evec)
    return "[]";

  if (array == val_ebin_s8)
    return "#s8[]";

  if (array == val_ebin_u8)
    return "#u8[]";

  if (array == val_ebin_s16)
    return "#s16[]";

  if (array == val_ebin_u16)
    return "#u16[]";

  if (array == val_ebin_s32)
    return "#s32[]";

  if (array == val_ebin_u32)
    return "#u32[]";

  if (array == val_ebin_s64)
    return "#s64[]";

  if (array == val_ebin_f64)
    return "#f64[]";

  __builtin_unreachable();
}

long validate_array_index( const char *fname, long n, value_t array ) {
  require( fname,
	   !is_empty(array),
	   "%ld out of bounds for %s",
	   n,
	   empty_array_str( array ) );

  if (n < 0) {
    require( fname,
	     -n < (long)alength(array),
	     "%ld out of bounds for array of size %zu",
	     n,
	     alength(array) );

    return n + alength(array);
  }

  require( fname,
	   n < (long)alength(array),
	   "%ld out of bounds for array of size %zu",
	   n,
	   alength(array) );

  return n;
}

value_t resize_vector( value_t vec, size_t n ) {
  if ( is_empty(vec))
    return vector( n, NULL );

  push( vec );

  size_t n_alloc = calc_array_size( n );
  size_t n_copy  = min( n, alength(vec) ) * sizeof(value_t);

  void *spc = allocate_words( n_alloc );
  
  vec = pop();

  memcpy( spc, adata( vec ), n_copy );

  adata( vec ) = spc;
  asize( vec ) = n_alloc;
  
  if ( n < alength(vec) )
    alength(vec) = n;

  return vec;
}

value_t vector_get( value_t vec, size_t n ) {
  return s64data( vec )[n];
}

value_t vector_set( value_t vec, size_t n, value_t x ) {
  s64data( vec )[n] = x;
  return x;
}

value_t vector_put( value_t vec, value_t x ) {
  if (is_empty(vec)) {
    push( x );
    vec = vector( 1, NULL );
    x = pop();
  }

  s64data(vec)[alength(vec)++] = x;

  if (alength(vec) == asize(vec))
    vec = resize_vector(vec, asize(vec) );

  return vec;
}

value_t vector_get_s( const char *fname, value_t *vec, long n ) {
  n = validate_array_index( fname, n, *vec );
  return vector_get( *vec, n );
}

value_t vector_set_s( const char *fname, value_t *vec, long n, value_t x) {
  n = validate_array_index( fname, n, *vec );
  return vector_set( *vec, n, x );
}

value_t vector_put_s( const char *fname, value_t *vec, value_t x) {
  (void)fname;
  *vec = vector_put(*vec, x );
  return *vec;
}

Ctype_t get_Ctype( value_t bin ) {
  if (is_immediate(bin))
    return cval(bin);

  return ob_Ctype(bin);
}

value_t resize_binary( value_t bin, size_t n ) {
  if (is_empty(bin))
    return binary(n, get_Ctype(bin), NULL );
  
  push( bin );

  size_t elsize = Ctype_size( get_Ctype( bin ) );

  size_t n_alloc = calc_array_size( n );
  size_t n_copy  = min( n, alength(bin) ) * elsize;

  void *spc = allocate_words( n_alloc );

  bin = pop();

  memcpy( spc, adata( bin ), n_copy );

  adata( bin ) = spc;

  return bin;
}

fixnum_t binary_get( value_t bin, size_t n ) {
  assert( !is_empty(bin) );
  Ctype_t ct = get_Ctype(bin);

  switch (ct) {
  case C_sint8:   return s8data(bin)[n];
  case C_uint8:   return u8data(bin)[n];
  case C_sint16:  return s16data(bin)[n];
  case C_uint16:  return u16data(bin)[n];
  case C_sint32:  return s32data(bin)[n];
  case C_uint32:  return u32data(bin)[n];
  case C_sint64:  return s64data(bin)[n];
  case C_float64: return (long)f64data(bin)[n];
  }

  __builtin_unreachable();
}


fixnum_t binary_set( value_t bin, size_t n, fixnum_t f) {
  assert( !is_empty(bin) );
  Ctype_t ct = get_Ctype(bin);
  assert( fixnum_fits(f, ct) );

  switch (ct) {
  case C_sint8:  s8data(bin)[n]  = f; break;
  case C_uint8:  u8data(bin)[n]  = f; break;
  case C_sint16: s16data(bin)[n] = f; break;
  case C_uint16: u16data(bin)[n] = f; break;
  case C_sint32: s32data(bin)[n] = f; break;
  case C_uint32: u32data(bin)[n] = f; break;
  case C_sint64: s64data(bin)[n] = f; break;
  case C_float64:f64data(bin)[n] = (double)f; break;
  }

  return f;
}

value_t binary_put( value_t bin, fixnum_t x ) {
  if (is_empty(bin)) {
    push( fixnum(x) );
    bin = binary( 1, cval(bin), NULL );
    x = ival( pop() );
  }

  binary_set(bin, alength(bin)++, x);

  if (alength(bin) == asize(bin))
    bin = resize_binary(bin, asize(bin) );

  return bin;
}

value_t binary_get_s( const char *fname, value_t *bin, long n ) {
  n = validate_array_index(fname, n, *bin );
  return binary_get( *bin, n );
}

value_t binary_set_s( const char *fname, value_t *bin, long n, value_t f ) {
  n = validate_array_index( fname, n, *bin );
  fixnum_t v = tofixnum(fname, f);
  Ctype_t ct = get_Ctype(*bin);
  require( fname,
	   fixnum_fits(v, ct),
	   "%ld not between %ld and %ld",
	   v,
	   Ctype_min_int[ct],
	   Ctype_max_int[ct] );

  v = binary_set( *bin, n, v );
  return fixnum(v);
}

value_t  binary_put_s( const char *fname, value_t *bin, value_t f) {
  fixnum_t x = tofixnum(fname, f);
  Ctype_t ct = get_Ctype(*bin);
  require( fname,
	   fixnum_fits(x, ct),
	   "%ld not between %ld and %ld",
	   x,
	   Ctype_min_int[ct],
	   Ctype_max_int[ct] );
  
  *bin = binary_put(*bin, x );
  return *bin;
}


value_t resize_string( value_t str, size_t n ) {
  if ( is_empty(str)) {
    string_s( n, NULL );
    return pop();
  }

  push( str );

  size_t n_alloc = calc_array_size( n );
  size_t n_copy  = min( n, alength(str) );

  char *spc = allocate( n_alloc + 1 );

  str = pop();

  memcpy( spc, adata( str ), n_copy );

  adata( str ) = spc;
  asize( str ) = n_alloc;
  
  if ( n < alength(str) )
    alength(str) = n;

  return str;
}

char string_get( value_t str, size_t n ) {
  return s8data( str )[n];
}

char string_set( value_t str, size_t n, char c ) {
  s8data( str )[n] = c;
  return c;
}

value_t string_put( value_t str, char c ) {
  if (is_empty(str)) {
    char buf[2] = { c, '\0' };
    return string( buf );
  }

  s8data(str)[alength(str)++] = c;

  if (alength(str) == asize(str))
    str = resize_string(str, asize(str) );

  return str;
}

value_t string_get_s( const char *fname, value_t *str, long n ) {
  n = validate_array_index( fname, n, *str );
  n = string_get( *str, n );
  return character( n );
}

value_t string_set_s( const char *fname, value_t *str, long n, value_t x) {
  n = validate_array_index( fname, n, *str );
  char c = tocharacter( fname, x );
  return string_set( *str, n, c );
}

value_t string_put_s( const char *fname, value_t *str, value_t x) {
  char c = tocharacter( fname, x );
  *str = string_put(*str, c );
  return *str;
}

// predicates -----------------------------------------------------------------
mk_type_p(vector)
mk_type_p(binary)
mk_type_p(string)

// safecasts ------------------------------------------------------------------
mk_safe_cast(vector, vector_t*, pval, true)
mk_safe_cast(binary, binary_t*, pval, true)
mk_safe_cast(string, string_t*, pval, true)

// methods --------------------------------------------------------------------
size_t vector_prin(FILE *ios, value_t x) {
  size_t out = fprintf( ios, "[" );

  if (!is_empty(x)) {
    value_t *vals = adata(x);
    size_t cap = alength(x);

    for (size_t i=0; i<cap; i++) {
      out += r_prin( ios, vals[i] );

      if (i+1 < cap)
	out += fprintf( ios, " " );
    }
  }

  return out + fprintf( ios, "]" );
}

size_t binary_prin( FILE *ios, value_t x) {
  Ctype_t ctype = get_Ctype(x);
  const char *fmt = Ctype_fmt[ctype];
  size_t out = fprintf( ios, "#%s[", Ctype_names[ctype] );

  if (!is_empty(x)) {
    size_t cap = alength(x);

    for (size_t i=0; i<cap; i++) {
      fixnum_t fx  = binary_get( x, i );
      out         += fprintf( ios, fmt, fx );

      if (i+1 < cap)
	out += fprintf( ios, " " );
    }
  }

  return out + fprintf( ios, "]" );
}

size_t string_prin(FILE *ios, value_t x) {
  if (x == val_estr)
    return fprintf( ios, "\"\"" );
  
  return fprintf( ios, "\"%s\"", s8data(x) );
}

int vector_order( value_t x, value_t y ) {
    size_t xn = alength(x), yn = alength(y);
    value_t *xa = adata(x), *ya = adata(y);
    int o;

    for (size_t i=0; i<min(xn, yn); i++)
      if ((o=r_order(xa[i], ya[i])))
	return o;

    return 0 - (xn < yn) + (xn > yn);
}

int binary_order( value_t x, value_t y ) {
  int o;
    Ctype_t xC = get_Ctype(x), yC = get_Ctype(y);

    if ((o=ord_uint(xC, yC)))
      return o;
    
    size_t Cs = Ctype_size( xC );
    
    size_t xn = alength(x), yn = alength(y);
    
    if ((o=memcmp(adata(x), adata(y), min(xn, yn) * Cs)))
      return o;
    
    return 0 - (xn < yn) + (xn > yn);
}

int string_order( value_t x, value_t y ) {
    return strcmp( s8data(x), s8data(y) );
}

size_t vector_sizeof( value_t x ) {
  if (is_empty(x))
    return 0;
  
  return sizeof(vector_t) + alength(x) * sizeof(value_t);
}

size_t string_sizeof( value_t x ) {
  if (is_empty(x))
    return 0;
  
  return sizeof(string_t) + alength(x);
}

size_t binary_sizeof( value_t x ) {
  if (is_empty(x))
    return 0;

  return sizeof(binary_t) + alength(x) * Ctype_size(ob_Ctype(x));
}

// builtins -------------------------------------------------------------------
static Ctype_t key_to_Ctype( const char *fname, value_t kw ) {
  require( fname,
	   is_keyword(kw),
	   "invalid option: not a keyword" );

  if (kw == r_kw_s8)  return C_sint8;
  if (kw == r_kw_u8)  return C_uint8;
  if (kw == r_kw_s16) return C_sint16;
  if (kw == r_kw_u16) return C_uint16;
  if (kw == r_kw_s32) return C_sint32;
  if (kw == r_kw_u32) return C_uint32;
  if (kw == r_kw_s64) return C_sint64;
  if (kw == r_kw_f64) return C_float64;

  error( fname,
	 "unkown option %s",
	 sname(kw) );
  
  return val_unbound;
}

r_predicate(vector)
r_predicate(binary)
r_predicate(string)
r_predicate(empty)

void r_builtin(vector) {
  vector_s( n, &Sref(n) );
}

void r_builtin(binary) {
  vargc( "binary", 1, n );

  value_t ctsym = Sref(n);
  Ctype_t ct = key_to_Ctype( "binary", ctsym );

  uchar buf[(n-1)*Ctype_size(ct)];
  size_t bufi = 0;

  for (size_t i=1; i<n; i++)
    bufi += fixnum_init("binary", Sref(n-i), ct, &buf[bufi] );

  binary_s( n-1, ct, buf );
  Stack[Sp-2] = Stack[Sp-1]; Sp--;
}

void r_builtin(string) {
  if (n == 0) {
    push( val_estr );
    return;
  }

  if (n == 1) {
    if (is_symbol(Sref(1))) {
      char *name = sname(Sref(1));
      n = strlen(name);
      char buf[n+1];
      strcpy(buf, name);
      string_s(n, buf);
      return;
      
    } else if (is_string(Sref(1))) {
      if (is_empty(Sref(1)))
	push( val_estr );

      else {
	char *name = adata(Sref(1));
	n = alength(Sref(1));
	char buf[n+1];
	strcpy(buf, name);
	string_s(n, buf);
	return;
      }
    }
  }
  
  
  char buf[n+1];

  for (size_t i=0; i<n; i++)
    buf[i] = tocharacter( "string", Stack[Sp-n+i]);

  string_s(n, buf);
}

void r_builtin(len) {
  argc( "len", n, 1 );
  type_t t = oargt( "len",
		    Stack[Sp-1],
		    5,
		    type_nil,
		    type_cons,
		    type_vector,
		    type_binary,
		    type_string );
  
  size_t l = 0;

  if ( t == type_nil )
    l  = 0;
  
  else if ( t == type_cons )
    l = list_length( Sref(1) );
  
  else
    l = alength(Stack[Sp-1]);

  push( fixnum( l ) );
}

void r_builtin(nth) {
  argc( "nth", n, 2 );
  long i = ival( argt( "nth", pop(), type_fixnum ) );
  type_t t = oargt( "nth", Stack[Sp-1], 4, type_cons, type_vector, type_binary, type_string );
  value_t v;

  if (t == type_cons)
    v = cons_nth_s( "nth", Sref(1), i );

  else if (t == type_vector)
    v = vector_get_s( "nth", &Sref(1), i );
  
  else if (t == type_binary)
    v = binary_get_s( "nth", &Sref(1), i );

  else
    v = string_get_s( "nth", &Sref(1), i );

  push( v );
}

void r_builtin(xth) {
  argc( "xth", n, 3 );
  value_t x = pop();
  long i = ival( argt( "xth", pop(), type_fixnum ) );
  type_t t = oargt( "xth", Stack[Sp-1], 4, type_cons, type_vector, type_binary, type_string );

  if ( t == type_cons )
    x = cons_xth_s( "xth", Sref(1), i, x );

  else if ( t == type_vector )
    x = vector_set_s( "xth", &Sref(1), i, x );

  else if ( t == type_binary )
    x = binary_set_s( "xth", &Sref(1), i, x );

  else
    x = string_set_s( "xth", &Sref(1), i, x );
  
  Stack[Sp-1] = x;
}

void r_builtin(put) {
  argc( "put", n, 2 );

  value_t x = pop();
  type_t t = oargt( "put", Stack[Sp-1], 3, type_vector, type_binary, type_string );

  if (t == type_vector)
    vector_put_s( "put", &Sref(1), x );
    
  else if (t == type_binary)
    binary_put_s("put", &Sref(1), x );

  else
    string_put_s("put", &Sref(1), x );
}


 // initialization -------------------------------------------------------------
void array_init( void ) {
  Typenames[type_vector] = "vector";
  Typenames[type_binary] = "binary";
  Typenames[type_string] = "string";

  prin_dispatch[type_vector] = vector_prin;
  prin_dispatch[type_binary] = binary_prin;
  prin_dispatch[type_string] = string_prin;

  order_dispatch[type_vector] = vector_order;
  order_dispatch[type_binary] = binary_order;
  order_dispatch[type_string] = string_order;

  sizeof_dispatch[type_vector] = vector_sizeof;
  sizeof_dispatch[type_binary] = binary_sizeof;
  sizeof_dispatch[type_string] = string_sizeof;

  builtin("vector", builtin_vector );
  builtin("binary", builtin_binary );
  builtin("string", builtin_vector );
 
  builtin("vector?", builtin_is_vector );
  builtin("binary?", builtin_is_binary );
  builtin("string?", builtin_is_string );
  builtin("empty?", builtin_is_empty );

  builtin( "len", builtin_len );
  builtin( "nth", builtin_nth );
  builtin( "xth", builtin_xth );
  builtin( "put", builtin_put );  
}
