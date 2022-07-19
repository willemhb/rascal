#include <string.h>

#include "numutils.h"

#include "array.h"
#include "runtime.h"
#include "object.h"

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

// accessors & utilities ------------------------------------------------------
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
  validate_array_index( fname, &n, *vec );
  return vector_get( *vec, n );
}

value_t vector_set_s( const char *fname, value_t *vec, long n, value_t x) {
  validate_array_index( fname, &n, *vec );
  return vector_set( *vec, n, x );
}

value_t vector_put_s( const char *fname, value_t *vec, value_t x) {
  
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

value_t vector_get( value_t vec, size_t n ) {
  return s64data( vec )[n];
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
  char *fmt = Ctype_fmt[ctype];
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
