#include <assert.h>
#include <stdarg.h>

#include "obj/cons.h"
#include "obj/type.h"

#include "vm/memory.h"

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
/* runtime methods */
void init_cons( cons_t *cons, value_t car, value_t cdr )
{
  cons->car = car;
  cons->cdr = cdr;
}

void trace_cons( object_t *object )
{
  cons_t *cons = (cons_t*)object;

  mark_value(cons->car);
  mark_value(cons->cdr);
}

/* constructors */
value_t make_cons( value_t car, value_t cdr )
{
  cons_t *out = (cons_t*)make_object(&ConsType);
  out->car    = car;
  out->cdr    = cdr;

  return tag_object(out);
}

value_t make_conses( size_t n, ... )
{
  value_t buf[n];
  
  va_list va; va_start(va, n);

  for ( size_t i=0; i<n; i++ )
    buf[i] = va_arg(va, value_t);

  va_end(va);

  return vec_to_cons(buf, n);
}

value_t make_list( size_t n, ... )
{
  if ( n == 0 )
    return NUL;

  value_t buf[n];

  va_list va; va_start(va, n);

  for ( size_t i=0; i<n; i++ )
    buf[i] = va_arg(va, value_t);

  va_end(va);

  return vec_to_list(buf, n);
}

value_t vec_to_cons( value_t *args, size_t n_args )
{
  assert(n_args >= 2);

  value_t out = args[n_args-1];

  save_values(1, &out);

  for (size_t i=n_args-1; i>0; i--)
    out = make_cons(args[i-1], out);

  return out;
}

value_t vec_to_list( value_t *args, size_t n_args )
{
  assert(n_args >= 1);

  value_t out = NUL;

  save_values(1, &out);

  for (size_t i=n_args; i>0; i--)
    out = make_cons(args[i-1], out);

  return out;
}

/* accessors */
#include "tpl/impl/record.h"

GET(cons, car, value_t);
SET(cons, car, value_t);
GET(cons, cdr, value_t);
SET(cons, cdr, value_t);

value_t get_cons_head( cons_t *cons )
{
  return get_cons_car(cons);
}

cons_t *get_cons_tail( cons_t *cons )
{
  return as_cons(get_cons_cdr(cons));
}

/* runtime */
void rl_obj_cons_init( void )
{
  gl_init_type(ConsType);
}

void rl_obj_cons_mark( void )
{
  gl_mark_type(ConsType);
}

void rl_obj_cons_cleanup( void ) {}

/* convenience */
size_t cons_len( cons_t *cons )
{
  size_t out = 0;

  while (cons)
    {
      out++;
      cons = get_cons_tail(cons);
    }

  return out;
}
