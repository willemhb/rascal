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
  fixnum_t     as_fixnum;
  bool_t       as_bool;

  nullptr_t    as_nul;
  stream_t     as_stream;
  native_t     as_native;

  object_t    *as_object;
} rl_data_t;

/* globals */
/* tags */
#define QNAN        0x7ff8000000000000ul
#define SIGNBIT     0x8000000000000000ul

/* value tags */
/* immediate tags */
#define REAL        0x0000000000000000ul
#define FIXNUM      0x7ffc000000000000ul
#define BOOL        0x7ffd000000000000ul
#define NUL         0x7ffe000000000000ul
#define STREAM      0x7fff000000000000ul
#define NATIVE      0xfffc000000000000ul
#define OBJECT      0xfffd000000000000ul

/* masks */
#define TAGMASK     0xffff000000000000ul
#define PTRMASK     (~TAGMASK)

/* values */
#define TRUE        0x7ffc000000000001ul
#define FALSE       0x7ffc000000000000ul

/* API */
datatype_t *rl_typeof( value_t x );
bool        rl_isa( value_t x, datatype_t *type );
void        mark_value( value_t x );
void       *toptr( value_t x );

/* runtime */
void rl_vm_value_init( void );
void rl_vm_value_mark( void );
void rl_vm_value_cleanup( void );

/* convenience */
#define as_value( x )  (((rl_data_t)(x)).as_value)

#define tagof( x )  ((x)&TAGMASK)
#define dataof( x ) ((x)&PTRMASK)
#define tag( x, t ) (dataof(as_value(x))|(t))

#endif
