#include "cons.h"
#include "memory.h"
#include "table.h"

// pair types
cons_t *new_cons( void )
{
  return alloc( sizeof(cons_t) );
}

void init_cons( cons_t *c, value_t car, value_t cdr )
{
  init_object( &c->object, CONS, false );

  c->car      = car;
  c->len      = 1;

  if (isProper(cdr))
    {
      c->isProper = true;
      c->tail     = as_ptr(cdr);

      if (c->tail)
	c->len += c->tail->len;
    }

  else
    {
      c->isProper = false;
      c->cdr      = cdr;

      if (is_cons(cdr) && !is_nul(cdr))
	c->len += cons_len(cdr);
    }
}

void mark_cons( object_t *object )
{
  cons_t *cons = (cons_t*)object;

  if (cons->isProper)
    {
      while (cons && obj_gcbits(cons) == WHITE)
	{
	  obj_gcbits(cons) = BLACK;
	  mark_val( cons->car );
	  cons = cons->tail;
	}
    }

  else
    {
      while (cons && obj_gcbits(cons) == WHITE)
	{
	  obj_gcbits(cons) = BLACK;
	  mark_val( cons->car );

	  if (is_cons(cons->cdr))
	      cons = as_cons(cons->cdr);

	  else
	    {
	      mark_val( cons->cdr );
	      cons = NULL;
	    }
	}
    }
}

hash_t hash_cons( object_t *object )
{
  cons_t  *cons  = (cons_t*)object;
  table_t *cache = mk_hasher_cache();
  hash_t   out   = CONS;

  if (cons->isProper)
    {
      
    }

  else
    {
      
    }

  return out;
}
