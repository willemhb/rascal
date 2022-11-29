#include <stdarg.h>

#include "obj/cons.h"
#include "obj/type.h"


/* commentary */

/* C types */

/* globals */
void trace_cons(object_t* object);

struct vtable_t ConsMethods =
  {
    .trace=trace_cons
  };

struct layout_t ConsLayout =
  {
    .vmtype=vmtype_objptr,
    .obsize=sizeof(cons_t)
  };

struct datatype_t ConsType =
  {
    {
      .obj= obj_init(&TypeType, sizeof(datatype_t), object_fl_static),
      .name="cons"
    },

    .methods=&ConsMethods,
    .layout=&ConsLayout
  };

/* API */
void trace_cons( object_t *object )
{
  cons_t *cons = (cons_t*)object;

  mark_value(cons->car);
  mark_value(cons->cdr);
}

value_t cons( value_t car, value_t cdr )
{
  cons_t *out = (cons_t*)make_object(&ConsType);
  out->car    = car;
  out->cdr    = cdr;

  return tag_object(out);
}

value_t cons_n( size_t n, ... )
{
  value_t out = NUL, buf[n];
  va_list va; va_start(va, n);

  for ( size_t i=0; i<n; i++ )
    buf[i] = va_arg(va, value_t);

  va_end(va);

  out = buf[n-1];

  for ( size_t i=n-1; i>0; i-- )
    out = cons(buf[i-1], out);

  return out;
}

/* runtime */
void rl_obj_cons_init( void )
{
  init_object(&ConsType.type.obj);
}

void rl_obj_cons_mark( void )
{
  mark_object(&ConsType.type.obj);
}
