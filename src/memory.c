#include <string.h>
#include <wchar.h>
#include <stdlib.h>

#include "hashing.h"
#include "memutils.h"
#include "numutils.h"
#include "strutils.h"

#include "memory.h"
#include "lispio.h"
#include "runtime.h"
#include "object.h"

// forward declarations -------------------------------------------------------
bool check_heap_overflow( heap_t *h, size_t n );
bool check_heap_resize( heap_t *h );
bool check_stack_grow( stack_t *s );
bool check_stack_shrink( stack_t *s );

void grow_stack( stack_t *s );
void shrink_stack( stack_t *s );

void grow_heap(heap_t *h);
void swap_heap(heap_t *h);

void *allocate_from( heap_t *h, size_t n_bytes );
void  deallocate_from( heap_t *h, void *space, size_t n_bytes );
void collect_garbage( void );

bool in_heap( void *p );
bool in_space( void *p );
bool in_swap( void *p );
bool is_allocated( value_t x );
bool is_moved( value_t x );

void    trace_array( value_t *a, size_t n );
value_t trace_pair( value_t x );
value_t trace_symbol( value_t x );
value_t trace_tuple( value_t x );
value_t trace_closure( value_t x );
value_t trace_map( value_t x );

// globals --------------------------------------------------------------------
// backing objects for globals ------------------------------------------------
heap_t         HeapObject;
exec_t         ExecObject;
symbol_table_t SymbolTableObject;

// internal method dispatch tables --------------------------------------------
value_t (*Trace[TYPE_PAD])(value_t x) = {
  [type_cons] = trace_pair,
  [type_symbol] = trace_symbol,
  [type_tuple] = trace_tuple,
  [type_closure] = trace_closure,
  [type_map] = trace_map
};

Ctype_t CTypes[TYPE_PAD] = {
  [type_port]    = C_sint32,
  [type_boolean] = C_sint32,
  [type_integer] = C_sint64,
  [type_flonum]  = C_double,
  [type_builtin] = C_sint16,

  [type_nil]     = C_pointer,
  [type_binary]  = C_pointer,
  [type_tuple]   = C_pointer,
  [type_map]     = C_pointer,
  [type_closure] = C_pointer,
  [type_cons]    = C_pointer,
  [type_symbol]  = C_pointer
};

// external utilities ---------------------------------------------------------
// misc -----------------------------------------------------------------------
size_t Ctype_size(Ctype_t ct)
{
  if (ct <= C_double)
    return 8;

  else if (ct <= C_float)
    return 4;

  else if (ct <= C_sint16)
    return 2;

  else
    return 1;
}

Ctype_t type_Ctype(type_t rt)
{
  return CTypes[rt];
}

bool in_heap( void *p )
{
  return in_space(p) || in_swap(p);
}

bool in_space( void *p )
{
  return withinp( p, Heap->space, Heap->available );
}

bool in_swap( void *p )
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

// internal utilities ---------------------------------------------------------
value_t trace_pair( value_t x )
{
  value_t *prev = &x, curr = x;

  while (is_cons(curr))
    {
      if (is_moved(curr))
	{
	  *prev = curr;
	  break;
	}

      curr = relocate( curr );
      prev = &cdr( curr );
      curr = cdr( curr );
    }

  return x;
}

value_t trace_symbol( value_t x )
{
  trace_array( &sym_bind(x), 3 );
  return x;
}

value_t move_tuple( value_t x )
{
  trace_array( &tup_space(x)[0], tup_length(x) );
  return x;
}

value_t trace_map( value_t x )
{
  if (is_node(x))
    trace_array( &car(x), 4 );

  else
    trace_array( &cdr(x), 1 );

  return x;
}

value_t trace_closure( value_t x )
{
  trace_array( &cdr(x), 5 );
  return x;
}

// memory management ---------------------------------------------------------
void *allocate( size_t n, bool global )
{
  if (global)
    return malloc_s( n );

  return allocate_from( Heap, n );
}

void deallocate( void *space, size_t n )
{
  deallocate_from( Heap, space, n );
}

bool is_moved( value_t x )
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

value_t collect( value_t x )
{
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

  type_t xt = lisp_type( x );
  x = relocate( x );

  if (Trace[xt])
    x = Trace[xt]( x );

  return x;
}

value_t relocate( value_t x )
{
  size_t xs   = lisp_size( x );
  void *new   = allocate( xs, false );

  memcpy( new, pval(x), xs );

  x = car( x ) = tagptr( new, hitag(x) );

  return x;
}

void *allocate_from(heap_t *h, size_t n_bytes)
{
  size_t n_alloc = aligned(n_bytes, h->alignment);
  void *out;

  // try to find suitable memory in free list
  value_t *prev = &h->free, node = h->free;

  while (is_cons(node))
    {
      size_t n_block = uval(car(node));

      if (n_block >= n_alloc)
	{
	  *prev   = cdr(node);
	  out     = &car(node);
	  n_alloc = n_block;
	  goto end;
	}

      prev = &cdr(node);
      node = cdr(node);
    }

  if (!h->collecting && check_heap_overflow(h, n_alloc))
    collect_garbage();

  out        = h->space;
  h->used   += n_alloc;

 end:
  memset( out, 0, n_alloc );
  return out;
}

void deallocate_from(heap_t *h, void *space, size_t n_bytes)
{
  ((cons_t*)space)->car = tagptr(n_bytes, INTEGER);
  value_t *prev = &h->free;
  value_t  node = h->free;

  while (is_cons(node))
    {
      if (uval(car(node)) < n_bytes)
	{
	  prev = &cdr(node);
	  node =  cdr(node);
	}
    }

  *prev = tagptr(space, LIST);
  ((cons_t*)space)->cdr = node;
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

void trace_sym_node( sym_node_t *st )
{
  if (st)
    {
      value_t tmp = st->entry.bind;
      tmp = relocate( tmp );
      st->entry.bind = tmp;

      trace_sym_node(st->left);
      trace_sym_node(st->right);
    }
}

void trace_symbol_table( symbol_table_t *st )
{
  trace_sym_node( st->root );
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
bool check_heap_overflow(heap_t *h, size_t n)
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

// exec object ----------------------------------------------------------------
void init_exec( exec_t *e )
{
  e->fp = e->bp = e->pc = e->cp = 0;

  e->open = NIL;
  e->vals = EMPTYTUP;
  e->code = EMPTYSTR;

  init_stack( &e->base );
}

void finalize_exec( exec_t *e )
{
  finalize_stack( &e->base );
}

// stack object ---------------------------------------------------------------
bool check_stack_grow( stack_t *s )
{
  return s->sp >= s->cap;
}

bool check_stack_shrink( stack_t *s )
{
  assert(s->sp > 0);
  return s->cap > N_STACK && s->cap / s->sp >= 2;
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
  s->cap     = N_HEAP;
  s->data = malloc_s(N_HEAP*sizeof(value_t));
}

void finalize_stack(stack_t *s)
{
  free_s(s->data);
}
// symbol table & node --------------------------------------------------------
sym_node_t *new_sym_node( size_t l )
{
  return malloc_s( sizeof(sym_node_t) + l + 1);
}

void init_sym_node( sym_node_t *sn, sym_node_t **sp, hash_t h )
{
  sn->left = sn->right = NULL;
  sn->name = &sn->entry.name[0];
  sn->hash = h;
  *sp = sn;
}

void free_sym_node( sym_node_t *sn )
{
  if (sn)
    {
      free_sym_node( sn->left );
      free_sym_node( sn->right );
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
  free_sym_node( st->root );
}

void init_symbol( symbol_t *s, char *n, size_t l, hash_t h, symbol_table_t *st )
{
  s->length = l;
  s->flags = 0;
  s->keyword = n[0] == ':';
  s->gensym = !in_heap(s);
  s->bound = false;

  s->bind = s->constant = UNBOUND;
  s->doc = EMPTYSTR;
  s->idno = ++st->num_symbols;
  s->hash = h;

  strcpy( s->name, n );
}

value_t intern_string( symbol_table_t *st, char *s )
{
  sym_node_t **node = &st->root;
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
      sym_node_t *n = new_sym_node( strsz );
      init_sym_node( n, node, h );
      init_symbol( &(*node)->entry, s, strsz, h, Symbols );
    }

  return tagptr( &(*node)->entry, SYMBOL );
}

// traversal table ------------------------------------------------------------
#define N_TRAVERSE 32
#define T_LOAD 0.625

traverse_t *new_traverse( void )
{
  return malloc_s( sizeof(traverse_t) );
}

void init_traverse( traverse_t *t )
{
  t->length = 0;
  t->cap = N_TRAVERSE;

  t->keys = malloc_s(sizeof(void*)*t->cap);
  t->locations = malloc_s(sizeof(int)*t->cap);
  t->values = malloc_s(sizeof(value_t)*t->cap);

  wmemset(t->locations, -1, t->cap );
}

void free_traverse( traverse_t *t )
{
  free(t->keys);
  free(t->locations);
  free(t->values);
  free(t);
}

void grow_traverse( traverse_t *t )
{
  // grow table size ----------------------------------------------------------
  t->cap *= 2;

  // reallocate buffers -------------------------------------------------------
  t->values = realloc_s(t->values, sizeof(value_t)*t->cap);
  t->keys = realloc_s(t->keys, sizeof(void*)*t->cap);
  t->locations = realloc_s(t->locations, sizeof(int)*t->cap);

  // rehash entries for new table size ----------------------------------------
  memset( t->keys, 0, sizeof(void*)*t->cap);
  wmemset( t->locations, -1, t->cap );

  for (int i=0; i<t->length; i++)
    {
      void *p = pval( t->values[i] );
      hash_t h = pointerhash( p );
      int l = h % t->cap;

      while (t->keys[l])
	{
	  l++;

	  if (l >= t->length)
	    l %= t->length;
	}

      t->keys[l] = p;
      t->locations[l] = i;
    }
}

int traverse_get( traverse_t *t, value_t x )
{
  void *p = pval(x);
  hash_t h = pointerhash(p);
  int i = h % t->cap, l;

  while ((l=t->locations[i++]) != -1)
    {
      if (t->keys[l] == p)
	break;

      if (i == t->cap)
	i %= t->cap;
    }

  return l;
}

int traverse_put( traverse_t *t, value_t x )
{
  
}

// initialization -------------------------------------------------------------
void memory_init( void )
{
  Exec        = &ExecObject;
  Stack       = &ExecObject.base;
  Heap        = &HeapObject;
  Symbols     = &SymbolTableObject;

  init_exec( Exec );
  init_heap( Heap );
  init_symbol_table( Symbols );
}
