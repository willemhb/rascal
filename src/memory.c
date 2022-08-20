#include <string.h>

#include "hashing.h"
#include "memutils.h"
#include "numutils.h"
#include "strutils.h"

#include "memory.h"
#include "lispio.h"

// forward declarations -------------------------------------------------------
bool_t         check_heap_overflow( heap_t *h, size_t n );
bool_t         check_heap_resize( heap_t *h );
bool_t         check_stack_grow( stack_t *s );
bool_t         check_stack_shrink( stack_t *s );
void           grow_stack( stack_t *s );
void           shrink_stack( stack_t *s );
void           grow_heap(heap_t *h);
void           swap_heap(heap_t *h);
void          *allocate_from(heap_t *h, size_t n_bytes);
value_t        intern_string( symbol_table_t *t, char *s );
void           collect_garbage( void );
size_t         calc_vector_size( size_t n_args, size_t *n_nodes );
value_t        relocate( value_t x );
bool_t         in_heap( void *p );
bool_t         in_space( void *p );
bool_t         in_swap( void *p );
bool_t         is_allocated( value_t x );
bool_t         is_moved( value_t x );

symbol_node_t *new_symbol_node( size_t l );
void           init_symbol_node( symbol_node_t *n, symbol_node_t **p, hash_t h );
void           finalize_symbol_node( symbol_node_t *n );
void           init_symbol( symbol_t *s, char *f, size_t l, hash_t h, symbol_table_t *st );

// globals --------------------------------------------------------------------
// backing objects for globals ------------------------------------------------
heap_t         HeapObject;
stack_t        StackObject;
symbol_table_t SymbolTableObject;

// external utilities ---------------------------------------------------------
// misc -----------------------------------------------------------------------
size_t Ctype_size(Ctype_t ct)
{
  if (ct <= C_utf8)
    return 1;

  else if (ct <= C_utf16)
    return 2;

  else if (ct <= C_float)
    return 4;

  else
    return 8;
}

size_t lisp_size(value_t x)
{
  type_t xt = lisp_type( x );

  switch (xt)
    {
    case type_type:        return sizeof(type_t);
    case type_none:        return 0;
    case type_any:         return sizeof(value_t);
    case type_boolean:     return sizeof(boolean_t);
    case type_character:   return sizeof(char_t);
    case type_integer:     return sizeof(integer_t);
    case type_flonum:      return sizeof(double);
    case type_builtin:     return sizeof(ushort);
    case type_closure:     return sizeof(closure_t);

    case type_vector:
      if (vec_height(x) == 8)
	return sizeof(vector_t);

      return
	sizeof(vector_t) + ceil_log2( vec_length(x) ) * sizeof(value_t);

    case type_binary:      return sizeof(binary_t) + bin_length(x);
    case type_symbol:      return sizeof(symbol_t) + sym_length(x) + 1; 
    case type_port:        return 4;
    case type_environment: return sizeof(environment_t);
    case type_pointer:     return sizeof(void*);
    case type_pair:
    case type_cons:
      return sizeof(cons_t);
    default:
      return sizeof(value_t);
    }
}

type_t lisp_type( value_t x )
{
  if (hitag(x) == IMMEDIATE)
    return (x>>32)&255;

  if (hitag(x) == OBJECT)
      return (car(x)>>32)&255;

  if (hitag(x) == FUNCTION)
    return (x&7) ? type_builtin : type_closure;

  if (hitag(x) == POINTER)
    return type_pointer;

  if (hitag(x) == PAIR)
    return type_pair;

  if (hitag(x) == CONS)
    return type_cons;

  if (hitag(x) == SYMBOL)
    return type_symbol;

  // invalid, serious error
  return type_none;
}

Ctype_t type_Ctype(type_t rt)
{
  switch (rt)
    {
    case type_type:
      return C_uint32;

    case type_boolean:
      return C_boolean;

    case type_character: case type_integer:
      return C_sint32;

    case type_flonum:
      return C_double;

    case type_port:
      return C_uint32;

    default:
      return C_pointer;
    }
}

bool_t in_heap( void *p )
{
  return in_space(p) || in_swap(p);
}

bool_t in_space( void *p )
{
  return withinp( p, Heap->space, Heap->available );
}

bool_t in_swap( void *p )
{
  size_t size = Heap->available / (1 + Heap->grew);

  return withinp( p, Heap->swap, size);
}

// stack manipulation ---------------------------------------------------------
index_t push( value_t x )
{
  index_t out        = Stack->sp++;
  Stack->data[out]   = x;

  if (check_stack_grow(Stack))
    grow_stack(Stack);

  return out;
}

value_t pop( void )
{
  assert(Stack->sp > 0);

  value_t out = Stack->data[--Stack->sp];

  if (check_stack_shrink(Stack))
    shrink_stack(Stack);

  return out;
}

void dup( void )
{
  value_t tos              = Stack->data[Stack->sp-1];
  Stack->data[Stack->sp++] = tos;

  if (check_stack_grow(Stack))
    grow_stack(Stack);
}

index_t pushn( size_t n )
{
  index_t out = Stack->sp;

  Stack->sp  += n;

  if (check_stack_grow(Stack))
    grow_stack(Stack);

  return out;
}

value_t popn( size_t n )
{
  value_t out = Stack->data[Stack->sp-1];
  Stack->sp  -= n;

  if (check_stack_shrink(Stack))
    shrink_stack(Stack);

  return out;
}

// predicates -----------------------------------------------------------------
ht_pred(pair, PAIR)
ht_pred(cons, CONS)
ht_pred(symbol, SYMBOL)
ht_pred(function, FUNCTION)
ht_pred(immediate, IMMEDIATE)
ht_pred(object, OBJECT)
ht_pred(pointer, POINTER)
wt_pred(boolean, BOOLEAN)
wt_pred(type, TYPE)

ot_pred(vector, VECTOR)
ot_pred(string, STRING)
ot_pred(binary, BINARY)

val_pred(nil, NIL)

 
bool_t is_list( value_t x )
{
  return is_nil(x) || is_cons(x);
}

bool_t is_keyword( value_t x )
{
  return is_symbol(x) && sym_keyword(x);
}

bool_t is_gensym( value_t x )
{
  return is_symbol(x) && sym_gensym(x);
}

// internal utilities ---------------------------------------------------------
size_t calc_vector_size(size_t n, size_t *n_nodes)
{ 
  if (n == 0)
    return 0;

  *n_nodes   = 1;
  size_t out = sizeof(vector_t);
  
  do
    {
      size_t n_full  = n / N_VECTOR;
      size_t n_extra = nextipow2( n % 32 );
      size_t n_level = n_full + !!(n_extra);

      out      += (sizeof(vector_t) + sizeof(value_t)) * n_full + sizeof(vector_t) + n_extra;
      *n_nodes += (n=n_level);
    } while (n > N_VECTOR);

  return out;
}

// memory management ---------------------------------------------------------

void *allocate( size_t n, bool_t global )
{
  if (global)
    return malloc_s( n );

  return allocate_from( Heap, n );
}

bool_t is_moved( value_t x )
{
  assert(is_allocated(x));
  assert(in_heap(pval(x)));

  return in_space(pval(car(x)));
}

void trace_array( value_t *x, size_t n )
{
  value_t buf[n];
  memcpy( buf, x, sizeof(value_t) * n);

  for (size_t i=n; i>0; i--)
    {
      buf[i-1] = relocate(buf[i-1]);
      x[i-1]   = buf[i-1];
    }
}

value_t relocate( value_t x )
{
  assert(hitag(x) != HEADER);

  if (!is_allocated(x))
    {
      if (is_port(x))
	Ports->opened[ival(x)]->refcount++;

      return x;
    }

  if (!in_heap(pval(x)))
    return x;

  if (is_moved(x))
    return car(x);

  type_t xt   = lisp_type( x );
  size_t xs   = lisp_size( x );
  void *new   = allocate( xs, false );

  memcpy( new, pval(x), xs );

  x = car( x ) = tagptr( new, hitag(x) );

  if (xt == type_cons || xt == type_pair)
      trace_array( pval(x), 2 );

  else if (xt == type_symbol)
      trace_array( &sym_bind(x), 1 );

  else if (xt == type_closure)
      trace_array( &clo_name(x), 4 );

  else if (xt == type_environment)
      trace_array( &env_names(x), 3 );

  else if (xt == type_vector)
    if (vec_height(x) == 8)
      trace_array( &vec_data(x), 1 );
  
    else
      trace_array( &vec_cache(x), 1 + vec_length(x) );
  else
    {}

  return x;
}

void *allocate_from(heap_t *h, size_t n_bytes)
{
  size_t n_alloc = aligned(n_bytes, h->alignment);

  if (!h->collecting && check_heap_overflow(h, n_alloc))
    collect_garbage();

  void *out  = h->space;
  h->used   += n_alloc;

  return out;
}

void collect_garbage( void )
{
  Heap->collecting = true;

  grow_heap( Heap );
  swap_heap( Heap );
  
  trace_symbol_table( Symbols );
  trace_stack( Stack );
  trace_gc_frames( Saved );
  trace_ios_map( Ports );

  Heap->grow = check_heap_resize( Heap );

  Heap->collecting = false;
}

void trace_symbol_node( symbol_node_t *st )
{
  if (st)
    {
      value_t tmp = st->entry.bind;
      tmp = relocate( tmp );
      st->entry.bind = tmp;

      trace_symbol_node(st->left);
      trace_symbol_node(st->right);
    }
}

void trace_symbol_table( symbol_table_t *st )
{
  trace_symbol_node( st->root );
}

void trace_stack( stack_t *s )
{
  for (size_t i=0; i<s->sp; i++)
    {
      value_t tmp = s->data[i];
      tmp = relocate(tmp);
      s->data[i] = tmp;
    }
}

void grow_heap(heap_t *h)
{
  if (h->grow)
    {
      h->available *= 2;
      h->swap       = realloc_s(h->swap, h->available);
      h->grow       = false;
      h->grew       = true;
    }

  else if (h->grew)
    {
      h->swap       = realloc_s(h->swap, h->available);
      h->grow       = false;
      h->grew       = false;
    }

  memset(h->swap, 0, h->available);
}

void swap_heap(heap_t *h)
{
  void *tmp = h->space;
  h->space  = h->swap;
  h->swap   = tmp;
  h->used   = 0;
}

// internal objects -----------------------------------------------------------
// heap object ----------------------------------------------------------------
bool_t check_heap_overflow(heap_t *h, size_t n)
{
  return !h->collecting && h->used + n >= h->available;
}

void init_heap(heap_t *h)
{
  h->used      = 0;
  h->alignment = sizeof(cons_t);
  h->available = N_HEAP*h->alignment;
  h->grow = h->grew = h->collecting = false;
  h->space = malloc_s(h->available);
  h->swap = malloc_s(h->available);
}

void finalize_heap(heap_t *h)
{
  free_s(h->space);
  free_s(h->swap);
}

// stack object ---------------------------------------------------------------
bool_t check_stack_grow( stack_t *s )
{
  return s->sp >= s->cap;
}

bool_t check_stack_shrink( stack_t *s )
{
  assert(s->sp > 0);
  return s->cap > s->cap_min && s->cap / s->sp >= 2;
}

void grow_stack(stack_t *s)
{
  s->cap  *= 2;
  s->data  = realloc_s(s->data, s->cap*sizeof(value_t));
}

void shrink_stack(stack_t *s)
{
  s->cap  /= 2;
  s->data  = realloc_s(s->data, s->cap*sizeof(value_t));
}

void init_stack(stack_t *s)
{
  s->sp      = 0;
  s->fp      = 0;
  s->cap_min = N_HEAP;
  s->cap     = N_HEAP;
  s->data = malloc_s(N_HEAP*sizeof(value_t));
}

void finalize_stack(stack_t *s)
{
  free_s(s->data);
}
// symbol table & node --------------------------------------------------------
symbol_node_t *new_symbol_node( size_t l )
{
  return malloc_s( sizeof(symbol_node_t) + l + 1);
}

void init_symbol_node( symbol_node_t *sn, symbol_node_t **sp, hash_t h )
{
  sn->left = sn->right = NULL;
  sn->name = &sn->entry.name[0];
  sn->hash = h;
  *sp = sn;
}

void free_symbol_node( symbol_node_t *sn )
{
  if (sn)
    {
      finalize_symbol_node( sn->left );
      finalize_symbol_node( sn->right );
      free_s( sn );
    }
}

void init_symbol_table( symbol_table_t *st )
{
  st->num_symbols = 0;
  st->root        = NULL;
}

void finalize_symbol_table( symbol_table_t *st )
{
  free_symbol_node( st->root );
}

value_t intern_string( symbol_table_t *st, char *s )
{
  symbol_node_t **node = &st->root;
  size_t strsz = strlen(s)+1;
  hash_t h     = strhash( s );

  while (*node)
    {
      int o = ord_ulong( h, (*node)->hash ) ? : strcmp( s, (*node)->name );

      if (o < 0)
	node = &(*node)->left;

      else if (o > 0)
	node = &(*node)->right;

      else
	break;
    }

  if (*node == NULL)
    {
      symbol_node_t *n = new_symbol_node( strsz );
      
      init_symbol_node( n, node, h );
      init_symbol( &n->entry, s, strsz, h, st );
    }

  return tagptr( &(*node)->entry, SYMBOL );
}

// objects --------------------------------------------------------------------
// symbol ---------------------------------------------------------------------
void init_symbol( symbol_t *s, char *f, size_t l, hash_t h, symbol_table_t *st )
{
  s->length   = l;
  s->type     = type_symbol;
  s->constant = false;
  s->bound    = false;
  s->bind     = UNBOUND;
  s->tag      = LOHEADER;
  
  strcpy( s->name, f );

  s->hash     = h;
  s->idno     = ++st->num_symbols;
  s->keyword  = f[0] == ':';
  s->gensym   = in_heap(s);
}

value_t symbol( char *name, bool_t interned )
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

// pairs & cons ---------------------------------------------------------------
void init_cons( cons_t *c, value_t car, value_t cdr )
{
  c->car = car;
  c->cdr = cdr;
}

value_t cons( value_t car, value_t cdr )
{
  preserve( 2, &car, &cdr );

  cons_t *new = allocate( sizeof(cons_t), false );
  init_cons( new, car, cdr );

  if (is_nil(cdr) || is_cons(cdr))
    return tagptr(new, CONS);

  else
    return tagptr(new, PAIR);
}

// vectors --------------------------------------------------------------------
void init_vector( vector_t *v, uint_t l, uchar_t h, void *d )
{
  v->length = l;
  v->height = h;
  v->type   = type_vector;
  v->tag    = LOHEADER;

  if (h == 8) // root flag
    {
      v->data = tagptr( d, OBJECT );
    }

  else
    {
      v->cache  = NIL;
      memcpy( v->space, d, l * sizeof(value_t) );
    }
}

value_t vector( value_t *args, size_t n_args )
{
  size_t n_nodes;
  size_t total_alloc = calc_vector_size( n_args, &n_nodes );

  if (total_alloc == 0)
    return EMPTYVEC;

  /* NB: assume args is on the stack */
  vector_t *next_node = allocate( total_alloc, false );
  vector_t *last_node = NULL;

  value_t   node_buffer[n_nodes];
  size_t    vec_l  = n_args;
  size_t    node_i = 0;
  size_t    height = 0;

  do
    {
      size_t n_nodes = 0;

      while (n_args > 0)
	{
	  size_t n_local = min(n_args, 32ul);
	  n_args        -= n_local;

	  init_vector( next_node, n_local, height, args );

	  node_buffer[node_i++] = tagptr(next_node, OBJECT );
	  n_nodes++;

	  size_t off = sizeof(vector_t) + ceil_log2( n_local ) * sizeof(value_t);
	  last_node  = next_node;
	  next_node  = (vector_t*)((uchar*)next_node + off);
	}

      args   = &node_buffer[node_i-n_nodes];
      n_args = n_nodes;
      height++;
    } while ( n_args > N_VECTOR );

  init_vector( next_node, vec_l, 8, last_node);
  
  return tagptr( next_node, OBJECT );
}

value_t vec_ref(value_t vec, uint_t n)
{
  assert( n <= vec_length(vec));

  value_t out = vec_data(vec);

  for (uint_t idx=n&0x1f;n; n >>=5, idx=n&0x1f)
    out = vec_space(out)[idx];

  return out;
}

// initialization -------------------------------------------------------------
void memory_init( void )
{
  Stack       = &StackObject;
  Heap        = &HeapObject;
  Symbols     = &SymbolTableObject;

  init_stack( Stack );
  init_heap( Heap );
  init_symbol_table( Symbols );
}
