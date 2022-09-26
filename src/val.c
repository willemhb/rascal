#include <stdio.h>
#include <stdlib.h>

#include "val.h"
#include "obj.h"
#include "mem.h"
#include "rt.h"
#include "template/arr.h"
#include "utils/num.h"
#include "utils/arr.h"
#include "obj/port.h"

type_t typeof_val(val_t x)
{
  switch (val_tag(x))
    {
    case SMALL:     return small_type(x);
    case INTEGER:
    case ARITY:
    case POINTER:   return int_type;
    case OBJECT:    return as_obj(x)->type;
    default:        return real_type;
    }
}

// error methods (fills gaps in api table where the type is invalid)
int ord_error( val_t x, val_t y )
{
  (void)x;
  (void)y;
  
  printf( "Fatal: request for ord of invalid type.\n" );
  printf( "Inputs: %.16lux, %.16lux\n", x, y );
  exit(1);
}

hash_t hash_error( val_t x )
{
  type_t type = typeof_val(x);
  printf( "Fatal: request for hash of invalid type %du.\n", type );
  printf( "Input: %.16lux", x );
  exit(1);
}

arity_t prin_error( obj_t *io, val_t x )
{
  printf( "Fatal: request for hash of invalid type %du.\n", typeof_val(x) );
  printf( "Inputs: %px, %.16lux", io, x );
  exit( 1 );
}

val_t call_error( val_t x, val_t *args, arity_t n )
{
  printf( "Fatal: request for hash of invalid type %du.\n", typeof_val(x) );
  printf( "Inputs: %.16lux, %p, %du.\n", x, args, n );
  exit( 1 );
}

// fallback methods
int ord_fallback( val_t x, val_t y )
{
  return cmp( x, y );
}

hash_t hash_fallback( val_t x )
{
  return hash_int( x );
}

arity_t prin_fallback( obj_t *io, val_t x )
{
  port_t *p   =  (port_t*)io;
  FILE   *ios = p->stream;
  type_t  xt  = typeof_val(x);
  char_t *xn  = ValApis[xt].name;
  arity_t out = fprintf( ios, "<%s>", xn );

  return out;
}

val_t call_fallback( val_t fn, val_t *args, arity_t n )
{
  (void)args;
  (void)n;

  type_error( fn, func_type );
}

// dispatchers (prin and )
int ord_vals(val_t x, val_t y)
{
  if (x == y)
    return 0;
  
  type_t xt = typeof_val(x), yt = typeof_val(y);

  if (xt != yt)
    return cmp( xt, yt );

  return ValApis[xt].ord( x, y );
}

hash_t hash_val( val_t x )
{
  type_t xt = typeof_val(x);
  
  return ValApis[xt].hash(x);
}

// alist implementation
static const size_t vals_min_cap = 8;


PAD_STACK_SIZE(vals, val_t)
NEW_ARRAY(vals, val_t)
INIT_ARRAY(vals, val_t)
MARK_ARRAY(vals)
FINALIZE_ARRAY(vals, val_t)
RESIZE_ARRAY(vals, val_t)

// initialization
void val_init( void )
{
  static char_t *type_names[] =
    {
      "invalid",

      "none",  "any",  "int",  "real", "bool", "chr",
      "atom",  "nil",  "cons", "func", "port", "str",
      "table", "code", "clo",  "envt"
    };
  
  ValApis[0] = (val_api_t) {
    .name="invalid",
    .hash=hash_error,
    .ord=ord_error,
    .prin=prin_error,
    .call=call_error
  };
  
  for (type_t t=1; t<num_val_types; t++)
    {
      ValApis[t] = (val_api_t)
	{
	  .name = type_names[t],
	  .hash = hash_fallback,
	  .ord  = ord_fallback,
	  .prin = prin_fallback,
	  .call = call_fallback
	};
    }
}
