#ifndef rascal_memory_h
#define rascal_memory_h

#include <assert.h>

#include "memutils.h"

#include "common.h"

// internal values -------------------------------------------------------------
#define N_STACK   4096
#define N_HEAP    8192
#define RESIZE_F  0.625
#define GROW_F    2.0

// C types --------------------------------------------------------------------
struct stack_t
{
  value_t *data;

  uint_t sp, cap;
};

struct exec_t
{
  stack_t base;

  uint_t fp, bp, pc, cp;

  value_t open; // list of open upvalues
  value_t vals; // tuple of saved 
  value_t code; // currently executing bytecode
};

struct table_t
{
  long length, cap;

  void    **keys;
  long     *locations;
  hash_t   *hashes;
  value_t  *values;
};

struct heap_t
{
  size_t used, available, alignment;

  bool grow, grew, collecting;

  value_t free;

  uchar *space, *swap;
};

struct gc_frame_t
{
  size_t      length;
  gc_frame_t *next;
  value_t    **saved;
};

// implementations ------------------------------------------------------------
void init_heap(heap_t *h);
void finalize_heap(heap_t *h);

void init_stack(stack_t *s);
void finalize_stack(stack_t *s);

table_t *new_table( void );
void init_table( table_t *t );
void free_table( table_t *t );

// utilities ------------------------------------------------------------------
// misc -----------------------------------------------------------------------
size_t Ctype_size( Ctype_t ct );
Ctype_t type_Ctype( type_t ct );
bool in_heap( void *p );
bool in_space( void *p );
bool in_swap( void *p );

// stack manipulation ---------------------------------------------------------
index_t push( value_t x );
value_t pop( void );
void dup( void );
index_t pushn( size_t n );
value_t popn( size_t n );

// table api ------------------------------------------------------------------
long table_get( table_t *t, void *k, hash_t h, int (*compare)(void *x, void *y) );
long table_put( table_t *t, void *k, hash_t h, int (*compare)(void *x, void *y) );
bool table_has( table_t *t, void *k, hash_t h, int (*compare)(void *x, void *y) );

value_t intern_string( table_t *t, char *s );
value_t intern_address( table_t *t, void *p );

// memory management ----------------------------------------------------------
void *allocate( size_t n, bool global );
void  deallocate( void *spc, size_t n );
value_t collect( value_t x );
value_t relocate( value_t x );
void trace( value_t x );
void gc_frame_cleanup( gc_frame_t *f );

// macros ---------------------------------------------------------------------
#define RSP_GC_FRAME gc_frame_t __gc_frame__ __attribute__((cleanup(gc_frame_cleanup)))

#define preserve(n, vars...)					\
  value_t *__gc_frame_array__[(n)] = { vars };			\
  RSP_GC_FRAME = { (n), Saved, &__gc_frame_array__[0] };	\
  Saved = &__gc_frame__

#endif
