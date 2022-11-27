#ifndef rl_vm_value_h
#define rl_vm_value_h

#include "rascal.h"

/* commentary

   Basic value API (eg type querying, size querying, etc) */

/* C types */
// union of types a tagged value can be
typedef union
{
  value_t      as_value;
  real_t       as_real;
  stream_t     as_stream;
  object_t     as_object;
  array_t      as_array;
} rl_data_t;

/* globals */
/* tags */
#define QNAN        0x7ff8000000000000ul
#define SIGNBIT     0x8000000000000000ul

/* value tags */
/* immediate tags */
#define REAL        0x0000000000000000ul
#define NUL         0x7ffc000000000000ul
#define STREAM      0x7ffd000000000000ul
#define OBJECT      0x7ffe000000000000ul

/* masks */
#define TAGMASK     0xffff000000000000ul
#define PTRMASK     (~TAGMASK)

/* API */
type_t rl_typeof( value_t x );

/* convenience */
#define as_value( x ) (((rl_data_t)(x)).as_value)

#define tagof( x )  ((x)&TAGMASK)
#define dataof( x ) ((x)&PTRMASK)
#define tag( x, t ) (dataof(as_value(x))|(t))

#endif
