#include "pair.h"
#include "memory.h"
#include "array.h"
#include "port.h"
#include "prin.h"

// pair types
bool is_proper(value_t value)
{
  return is_nul(val)
    || (is_cons(val)
	&& flag_p(as_obj(val)->flags, cons_fl_proper));
}

bool is_list(value_t value)
{
  return is_nul(val) || is_cons(val);
}

OBJ_NEW(cons);

cons_t *new_conses(arity_t n)
{
  return alloc_vec(n, cons_t );
}

void init_cons( cons_t *cons, value_t car, value_t cdr )
{
  init_obj(&cons->obj, CONS, cons_fl_proper * is_proper(cdr) );
  cons->car    = car;
  cons->cdr    = cdr;
  cons->eltype = ANY;
  cons->len    = 1;

  if (is_cons(cdr))
    cons->len += as_cons(cdr)->len;
}

void init_conses( cons_t *cons, value_t *args, arity_t n )
{
  for (arity_t i=n-1; i>0; i--)
    {
      value_t pair = tag_ptr(cons+i-1, OBJ);
      value_t tail = args[i];
      value_t head = args[i-1];
      init_cons(cons+i-1, head, tail);
      args[i-1] = pair;
    }
}

void trace_cons( object_t *obj)
{
  cons_t *cons = (cons_t*)obj;

  while (!obj_black(cons))
    {
      obj_black(cons) = true;
      
      mark_val(cons_car(cons));
      
      if (is_cons(cons_cdr(cons)))
	cons = as_cons(cons_cdr(cons));

      else
	mark_val(cons_cdr(cons));
    }
}

void prin_list(stream_t *port, value_t value)
{
  port_prinf(port, "(" );

  while (is_cons(val))
    {
      lisp_prin(port, cons_car(val));
      val = cons_cdr(val);

      if (!is_list(val))
	port_prinf(port, " . ");

      else if (!is_nul(val))
	port_prinf(port, " ");
    }

  port_prinf(port, ")");
}
