#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <tgmath.h>

#include "hashing.h"
#include "numutils.h"
#include "strutils.h"

#include "object.h"
#include "memory.h"
#include "runtime.h"

// forward declarations -------------------------------------------------------
// size dispatch --------------------------------------------------------------
size_t symbol_sizeof(value_t x);
size_t tuple_sizeof(value_t x);
size_t binary_sizeof(value_t x);
size_t map_sizeof(value_t x);

// order dispatch -------------------------------------------------------------
order_t flo_order( value_t x, value_t y );
order_t int_order( value_t x, value_t y );
order_t port_order( value_t x, value_t y );
order_t bool_order( value_t x, value_t y );
order_t sym_order( value_t x, value_t y );
order_t list_order( value_t x, value_t y );
order_t tuple_order( value_t x, value_t y );
order_t map_order( value_t x, value_t y );
order_t bin_order( value_t x, value_t y );
order_t builtin_order( value_t x, value_t y );
order_t closure_order( value_t x, value_t y );

// map implementation helpers -------------------------------------------------
double  floga(size_t s);
size_t  node_height( value_t n );
size_t  node_size( value_t n );
bool    height_balanced( size_t h, size_t w );
bool    weight_balanced( size_t n, size_t l, size_t r );
value_t map_insert( value_t *m, value_t k, node_t *n );
value_t map_delete( value_t *m, value_t k );
value_t map_assoc( value_t m, value_t k );

// globals --------------------------------------------------------------------
// internal method dispatch tables --------------------------------------------
size_t (*Sizeof[TYPE_PAD])(value_t) =
  {
    [type_tuple] = tuple_sizeof,
    [type_binary] = binary_sizeof,
    [type_symbol] = symbol_sizeof,
    [type_map] = map_sizeof
  };

order_t (*Order[TYPE_PAD])(value_t,value_t) =
  {

    [type_nil]     = list_order,

    [type_integer] = int_order,
    [type_flonum]  = flo_order,
    [type_port]    = port_order,
    [type_boolean] = bool_order,
    [type_builtin] = builtin_order,
    [type_closure] = closure_order,
    [type_symbol]  = sym_order,
    [type_cons]    = list_order,
    [type_map]     = map_order,
    [type_binary]  = bin_order,
    [type_tuple]   = tuple_order
  };

size_t TypeSizes[TYPE_PAD] =
  {
    [type_port] = sizeof(port_t),
    [type_nil] = 8,
    [type_boolean] = sizeof(boolean_t),
    [type_integer] = sizeof(integer_t),
    [type_flonum] = sizeof(flonum_t),
    [type_builtin] = sizeof(ushort_t),
    [type_closure] = sizeof(closure_t),
    [type_cons] = sizeof(cons_t),
    [type_tuple] = sizeof(tuple_t),
    [type_binary] = sizeof(binary_t),
    [type_map] = sizeof(map_t),
    [type_symbol] = sizeof(symbol_t)
  };

// internal methods -----------------------------------------------------------
size_t tuple_sizeof( value_t x )
{
  if (x == EMPTYTUP)
    return 8;
    
  return sizeof(tuple_t) + tup_length(x) * sizeof(value_t);
}

size_t symbol_sizeof( value_t x )
{
  if (x == UNBOUND)
    return 8;

  return sizeof(symbol_t) + sym_length(x) + 1;
}

size_t binary_sizeof( value_t x )
{
  if (x == EMPTYSTR)
    return 8;

  return sizeof(binary_t) + (bin_length(x) + bin_string(x)) * Ctype_size(bin_Ctype(x));
}

size_t map_sizeof( value_t x )
{
  if (x == EMPTYMAP)
    return 8;
  
  if (is_integer(car(x)))
    return sizeof(map_t);

  else
    return sizeof(node_t);
}

// utilities -----------------------------------------------------------------


// initializers ---------------------------------------------------------------
void init_cons( cons_t *c, value_t car, value_t cdr )
{
  c->car = car;
  c->cdr = cdr;
}

void init_binary( binary_t *b, uint_t n, Ctype_t C, bool str, void *c)
{
  b->length = n;
  b->Ctype = C;
  b->string = str;
  b->flags = 0;
  b->tag = INTEGER>>48;

  if (c)
    memcpy(b->space, c, (n+str)*Ctype_size(C));
}

void init_tuple( tuple_t *t, uint_t n, value_t *v )
{
  t->length = n;
  t->flags = 0;
  t->tag = INTEGER>>48;

  if  (v)
    memcpy(t->space, v, n*sizeof(value_t));
}

void init_node( node_t *n, value_t m, value_t *p, value_t k, value_t b )
{
  n->left = n->right = EMPTYMAP;
  n->key = k;
  n->bind = b;
  *p = tagptr(n, MAP);

  map_length(m)++;
}

void init_map( map_t *m, uint_t n, value_t *v, node_t *s )
{
  m->length = 0;
  m->flags = 0;
  m->tag = INTEGER >> 48;
  m->data = EMPTYMAP;

  value_t mt = tagptr(m, MAP );

  for (uint_t i=0; i<n; i+= 2)
    {
      value_t loc    = map_insert( &mt, v+i, &s );
      node_bind(loc) = v[i+1];
    }

  
  uint_t used = m->length, nk = n / 2, unused = nk - used;

  deallocate( s, sizeof(node_t) * unused );
}

// constructors ---------------------------------------------------------------
value_t cons( value_t car, value_t cdr )
{
  preserve( 2, &car, &cdr );

  cons_t *new = allocate( sizeof(cons_t), false );
  init_cons( new, car, cdr );

  return tagptr(new, LIST);
}

// symbol ---------------------------------------------------------------------
value_t symbol( char *name, bool interned )
{
  static const char *gsfmt = "%s#%lu";
  if (interned)
    {
      assert(name);
      assert(strlen(name));
      return intern_string( Symbols, name );
    }

  if (name == NULL)
    name = "symbol";

  ulong_t sidno = Symbols->num_symbols+1;
  size_t  strsz = strlen(name)+1;
  size_t  bufsz = strsz+SAFE_NUMBER_BUFFER_SIZE+1;

  char buf[bufsz];
  snprintf( buf, bufsz, gsfmt, name, sidno );
  hash_t h = strhash( buf );

  memset( buf, 0, bufsz );

  snprintf( buf, strsz, "%s", name );

  symbol_t *out = allocate( sizeof(symbol_t)+strsz, false );

  init_symbol( out, buf, strsz, h, Symbols );
  return tagptr( out, SYMBOL );
}

// strings & binaries ---------------------------------------------------------
value_t string(char *chrs, size_t n)
{
  if (chrs == NULL || n == 0)
    return EMPTYSTR;

  char buf[n+1];

  strcpy(buf, chrs);

  string_t *new = allocate(sizeof(string_t) + (n + 1), false );
  init_binary( new, C_ascii, true, n, chrs );
  return tagptr( new, BINARY );
}

value_t gl_string(char *chrs, size_t n)
{
  if (chrs == NULL || n == 0)
    return EMPTYSTR;

  string_t *new = allocate(sizeof(string_t) + n + 1, true );
  init_binary( new, C_ascii, true, n, chrs );
  return tagptr( new, BINARY );
}

// tuples --------------------------------------------------------------------
value_t tuple( value_t *args, size_t n )
{
  if (n == 0)
    return EMPTYTUP;

  tuple_t *new = allocate( sizeof(tuple_t) + n*sizeof(value_t), false);
  init_tuple(new, n, args);
  return tagptr(new, TUPLE);
}

// accessors ------------------------------------------------------------------
value_t map_ref(value_t m, value_t k)
{
  value_t pair = map_assoc( m, k );

  if (pair == NIL)
    return UNBOUND;

  return cdr( pair );
}

// utilities ------------------------------------------------------------------
// external utilities ---------------------------------------------------------
type_t lisp_type(value_t x)
{ 
  if (hitag(x) == IMMEDIATE)
    return x>>32&255;

  if (hitag(x) == INTEGER)
    return type_integer;

  if (hitag(x) == FUNCTION)
    return (x&7) ? type_builtin : type_closure;

  if (hitag(x) == BINARY)
    return type_binary;

  if (hitag(x) == TUPLE)
    return type_tuple;

  if (hitag(x) == MAP)
    return type_map;

  if (hitag(x) == SYMBOL)
    return type_symbol;

  if (uval(x))
    return type_cons;

  return type_nil;
}

size_t lisp_size(value_t x)
{
  type_t xt  = lisp_type( x );

  if (Sizeof[xt])
    return Sizeof[xt](x);

  return TypeSizes[xt];
}

order_t lisp_order( value_t x, value_t y )
{
  if (x == y)
    return 0;

  type_t xt = lisp_type(x), yt = lisp_type(y);

  if (xt != yt)
    return ord_uint(xt, yt);

  return Order[xt](x, y);
}

// internal utilities ---------------------------------------------------------
order_t flo_order( value_t x, value_t y )
{
  return ord_double( fval(x), fval(y) );
}

order_t int_order( value_t x, value_t y )
{
  return ord_long( lval(x), lval(y) );
}

order_t port_order( value_t x, value_t y )
{
  return ord_int( ival(x), ival(y) );
}

order_t bool_order( value_t x, value_t y )
{
  if (x == FALSE)
    return -1 * (y != FALSE);

  if (y == FALSE)
    return 1;

  return 0;
}

order_t sym_order( value_t x, value_t y )
{
  if (x == UNBOUND)
    return -1 * (y != UNBOUND);

  if (y == UNBOUND)
    return 1;

  order_t o = ord_ulong( sym_hash(x), sym_hash(y) );

  if (!o)
    o = strcmp( sym_name(x), sym_name(y) );

  if (!o)
    o = ord_ulong( sym_idno(x), sym_idno(y) );

  return o;
}

order_t map_order(value_t x, value_t y)
{
  if (x == EMPTYMAP)
    return -1 * (y != EMPTYMAP);

  if (y == EMPTYMAP)
    return 1;

  
}

// map implementation utilities -----------------------------------------------
#define N_ALPHA 0.625
#define N_LOG 1.6

double floga( size_t s )
{
  return floor(log(s)/log(N_LOG));
}

bool height_balanced( size_t h, size_t s )
{
  return h <= floga(s) + 1;
}

bool weight_balanced( size_t n, size_t l, size_t r )
{
  return l <= N_ALPHA*n && r <= N_ALPHA*n;
}

size_t node_size( value_t n )
{
  if (n == EMPTYMAP)
    return 0;

  return 1
    + node_size(node_left(n))
    + node_size(node_right(n));
}

size_t node_height( value_t n )
{
  if (n == EMPTYMAP)
    return 0;

  return 1 + max( node_height(node_left(n)),
		  node_height(node_right(n)) );
}


size_t balance_nodes(value_t *l, size_t n)
{
  void linearize_nodes( value_t n, value_t *buf, size_t *bufp)
  {
    linearize_nodes(node_left(n), buf, bufp);
    buf[(*bufp)++] = n;
    linearize_nodes(node_right(n), buf, bufp);
  }

  value_t init_from_sort( value_t *buf, size_t start, size_t end)
  {
    if (start >= end)
	return EMPTYMAP;

    
    size_t mid = start + (end - start) / 2;

    value_t out     = buf[mid];
    node_left(out)  = init_from_sort(buf, start, mid);
    node_right(out) = init_from_sort(buf, mid+1, end);
    return out;
  }
  
  value_t buf[n]; size_t bufp = 0;

  linearize_nodes(*l, buf, &bufp);
  
}

bool went_left( value_t n )
{
  assert(is_node(n));
  return hitag(node_left(n)) == INTEGER;
}

bool went_right( value_t n )
{
  assert(is_node(n));
  return hitag(node_right(n)) == INTEGER;
}

value_t map_insert(value_t *m, value_t k, node_t *spc )
{
  value_t *out = NULL; bool found = false, init = false, balanced = true;

  preserve( 2, m, &k );

  if (!spc)
    spc = allocate(sizeof(node_t), false);

  else
    init = true;

  value_t n = map_data(*m), p = EMPTYMAP, tmp;

  size_t h0 = 0;

  order_t o = 0;
  
  while (n != EMPTYMAP)
    {
      o  = lisp_order( k, node_key(n) );
      h0++;

      if (o < 0)
	{
	  tmp = n;
	  n = node_left(tmp);
	  node_left(tmp) = tagptr(p, INTEGER);
	  p = tmp;
	}

      else if (o > 0)
	{
	  tmp = n;
	  n               = node_right(tmp);
	  node_right(tmp) = tagptr(p, INTEGER);
	  p               = tmp;
	}

      else
	{
	  found = true;
	  out = &cdr( node_key(n) );
	}
    }

  if (out == NULL)
    {
      init_node( spc, *m, o == -1 ? &node_left(p) : &node_right(p), k, NIL );
      out = &spc->key;
      balanced = height_balanced(h0, map_length(*m));
    }

  size_t s = 1, sl = 0, sr = 0;

  while (!balanced)
    {
      if (went_left(p))
	{
	  tmp = tagptr(node_left(p), MAP);
	  node_left(p) = tagptr(n, MAP);
	  sl = s;
	  sr = node_size(node_right(p));
	}

      else
	{
	  tmp = tagptr(node_right(p), MAP);
	  node_right(p) = tagptr(n, MAP);
	  sr = s;
	  sl = node_size(node_left(p));
	}

      s = 1 + sl + sr;

      if (!weight_balanced(s, sl, sr)) // p is a candidate scapegoat node
	{
	  
	}
    }

  while (p != EMPTYMAP)
    {
      if (went_left(p))
	{
	  tmp = tagptr(node_left(p), MAP);
	  node_left(p) = tagptr(n, MAP);
	}

      else // went right
	{
	  tmp = tagptr(node_right(p), MAP);
	  node_right(p) = tagptr(n, MAP);
	}

      n = tagptr(p, MAP);
      p = tmp;
    }

  if (found && !init)
    deallocate( spc, sizeof(node_t));
  
  return tagptr(out, LIST);
}
