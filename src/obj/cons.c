#include <assert.h>
#include <stdarg.h>

#include "repr.h"

#include "obj/cons.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
void init_cons( object_t *object, size_t n, void *ini );
void compare_conses( value_t x, value_t y );

field_t ConsFields[] =
  {
    OBJLAYOUT,
    { repr_val, true, true, offsetof(cons_t, car) },
    { repr_val, true, true, offsetof(cons_t, cdr) }
  };

type_t ConsType =
  {
    TYPEINIT(cons_t, "cons", ConsFields, 2),
    NULL,
    
  };

/* API */

value_t cons( value_t car, value_t cdr )
{
  cons_t *out    = (cons_t*)make_object(&ConsType, 2);
  out->car       = car;
  out->cdr       = cdr;
  out->obj.flags = cons_fl_proper * is_proper(cdr);

  return tag_object(out);
}


value_t cons_n( size_t n, ... )
{
  if ( n == 0 )
    return NUL;

  va_list va; va_start(va, n);
  
  if ( n == 1 )
    {
      value_t car = va_arg(va, value_t); va_end(va);
      return cons(car, NUL);
    }

  value_t buffer[n];

  for (size_t i=0; i<n; i++)
    buffer[i] = va_arg(va, value_t);

  va_end(va);

  value_t out = buffer[n-1];

  for (size_t i=n-1; i>0; i--)
    out = cons(buffer[i-1], out);

  return out;
}


value_t assoca( value_t key, value_t list )
{
  while ( is_cons(list) )
    {
      if ( key == cons_caar(list) )
	return cons_car(list);

      list = cons_cdr(list);
    }

  return NUL;
}

value_t assocp( value_t key, value_t list )
{
  while ( is_cons(list) )
    {
      if ( key == cons_car(list) )
	break;

      list = cons_cddr(list);
    }

  return list;
}
