#include <string.h>

#include "hashing.h"
#include "memutils.h"
#include "object.h"
#include "runtime.h"

static value_t intern( symbols_t **table, char *name ) {
  hash_t h = strhash( name );

  while (*table) {
    int o = ord_ulong( h, (*table)->base.hash );

    if (o == 0)
      o = strcmp( name, (*table)->base.name );

    if (o < 0)
      table = &(*table)->left;

    else if (o > 0)
      table = &(*table)->right;

    else
      break;
  }

  if (*table == NULL) {
    size_t l = strlen(name);
    symbols_t* loc = malloc_s( sizeof(symbols_t) + l + 1);
    symbol_t* new = &(loc->base);
    init_symbol( new, h, name, l );
    *table = loc;
  }

  return tagp( &(*table)->base, tag_boxed );
}

// constructors ---------------------------------------------------------------
value_t symbol( char *name ) {
  return intern( &Symbols, name );
}

value_t gensym( char *name ) {  
  static const char *gs_fmt = "%s#%lu";

  if (name == NULL)
    name = "symbol";

  size_t l = strlen(name);
  char namebuf[l+1], hashbuf[l+2+SAFE_NUMBER_BUFFER_SIZE];
  strcpy(namebuf, name);
  snprintf( hashbuf, l+2+SAFE_NUMBER_BUFFER_SIZE, gs_fmt, namebuf, Symcnt+1 );
  hash_t h = strhash( hashbuf );
  symbol_t *out = allocate( sizeof(symbol_t) + l + 1);
  init_symbol( out, h, namebuf, l );
  return tagp( out, tag_boxed );
}


value_t builtin( char *name, void (*callback)( size_t n ) ) {
  value_t sname = symbol( name );
  builtin_t *spc = malloc_s( sizeof(builtin_t ) );

  init_builtin( spc, sname, callback );
  return tagp( spc, tag_boxed );
}

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

value_t resize_vector( value_t vec, size_t n ) {
  if ( vec == val_evec )
    return vector( n, NULL );

  push( vec );

  size_t n_alloc = calc_array_size( n );
  size_t n_copy  = min( n, alength(vec) ) * sizeof(value_t);

  void *spc = allocate_words( n_alloc );

  vec = pop();

  memcpy( spc, adata( vec ), n_copy );

  adata( vec ) = spc;

  return vec;
}

value_t cons( value_t ca, value_t cd ) {
  push_s( "cons", cd );
  push_s( "cons", ca );

  cons_s( &Stack[Sp-1], &Stack[Sp-2] );

  return pop();
}

index_t cons_s( value_t *ca, value_t *cd ) {
  cons_t *out    = allocate_pairs(1);
  value_t tagged = tagp( out, tag_pair );

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

value_t binary( size_t n, Ctype_t ctype, void *args ) {
  if ( n == 0 )
    return val_ebin;

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
    push( val_ebin );
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

value_t resize_binary( value_t bin, size_t n ) {
  push( bin );

  size_t elsize = Ctype_size( obhead(bin).Ctype );

  size_t n_alloc = calc_array_size( n );
  size_t n_copy  = min( n, alength(bin) ) * elsize;

  void *spc = allocate_words( n_alloc );

  bin = pop();

  memcpy( spc, adata( bin ), n_copy );

  adata( bin ) = spc;

  return bin;
}

value_t fixnum( long x ) { return tagi( x, tag_fixnum ); }
value_t boolean( int x ) { return x ? val_true : val_nil; }

// utilities ------------------------------------------------------------------
type_t r_type( value_t x ) {
  switch (tag(x)) {
  case tag_immediate: return x>>24;
  case tag_boxed: return obhead(x).type >> 3;
  default: return tag(x);
  }
}


mk_tag_p(fixnum)
mk_tag_p(immediate)
mk_tag_p(boxed)

mk_type_p(symbol)
mk_type_p(builtin)
mk_type_p(closure)
mk_type_p(vector)
mk_type_p(binary)


bool is_cons( value_t x ) { return tag(x) == tag_pair; }
bool is_list( value_t x ) { return is_cons(x) || is_nil(x); }

mk_val_p(nil)

mk_val_p(true)
mk_val_p(false)

bool is_boolean( value_t x ) { return is_true(x) || is_false(x); }

size_t r_size( value_t x ) {
  switch (tag(x)) {
  case tag_fixnum: return 8;
  case tag_immediate: return 4;
  case tag_pair: return sizeof(cons_t);
  default: break;
  }

  size_t base = obhead(x).base_size;

  if (obhead(x).is_array) {
    size_t n_alloc = asize(x);
    size_t b_alloc = (n_alloc + !!(obhead(x).encoding)) * Ctype_size( obhead(x).Ctype );
    base += b_alloc;
  }

  return base;
}

int r_order( value_t x, value_t y ) {
  if (x == y)
    return 0;

  int xtag = tag(x), ytag = tag(y), o;
  type_t xtype = r_type( x ), ytype = r_type( y );
  
  if ((o=ord_uint(xtype, ytype)))
    return o;

  if ((o=ord_uint(xtag, ytag)))
    return o;

  if (xtag == tag_fixnum)
    return ord_long( ival(x), ival(y) );

  if (xtag == tag_immediate)
    return ord_int( cval(x), cval(y) );

  if (xtag == tag_pair) {
    while (is_cons(x) && is_cons(y)) {
      if ((o=r_order( car(x), car(y))))
	return o;

      x = cdr(x);
      y = cdr(y);
    }

    o = 0 - is_nil(x) + is_nil(y);

    return o ? : r_order( x, y );
  }

  if (xtype == type_builtin)
    return ord_ulong( x&~(value_t)7, y&~(value_t)7 );

  if (xtype == type_symbol) {
    if ((o = strcmp( assymbol(x)->name, assymbol(y)->name )))
      return o;

    return ord_ulong( assymbol(x)->idno, assymbol(y)->idno );
  }

  if (xtype == type_closure) {
    if ((o=r_order(asclosure(x)->code, asclosure(y)->code)))
      return o;

    if ((o=r_order(asclosure(x)->envt, asclosure(y)->envt)))
      return o;

    return r_order(asclosure(x)->vals, asclosure(y)->vals);
  }

  if (xtype == type_vector) {
    size_t xn = alength(x), yn = alength(y);
    value_t *xa = adata(x), *ya = adata(y);

    for (size_t i=0; i<min(xn, yn); i++)
      if ((o=r_order(xa[i], ya[i]))) return o;
      
    return 0 - (xn < yn) + (xn > yn);
  }

  if (xtype == type_binary) {
    Ctype_t xC = obhead(x).Ctype, yC = obhead(y).Ctype;

    if ((o=ord_uint(xC, yC)))
      return o;
    
    size_t Cs = Ctype_size( xC );
    
    size_t xn = alength(x), yn = alength(y);
    
    if ((o=memcmp(adata(x), adata(y), min(xn, yn) * Cs)))
      return o;
    
    return 0 - (xn < yn) + (xn > yn);
  }
  
  __builtin_unreachable();
}

// safecasts ------------------------------------------------------------------
mk_safe_cast(cons, cons_t*, pval)
mk_safe_cast(symbol, symbol_t*, pval)
mk_safe_cast(fixnum, fixnum_t, ival)
mk_safe_cast(boolean, boolean_t, ival)
mk_safe_cast(builtin, builtin_t*, pval)
mk_safe_cast(closure, closure_t*, pval)
mk_safe_cast(vector, vector_t*, pval)
mk_safe_cast(binary, binary_t*, pval)
