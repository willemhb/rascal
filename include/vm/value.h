#ifndef rl_vm_value_h
#define rl_vm_value_h

#include "rascal.h"

/* commentary

   Basic value API (eg type querying, size querying, etc) */

/* C types */
// union of types a tagged value can be
typedef union
{
  rl_value_t as_value;
  rl_real_t as_real;
  rl_fixnum_t as_fixnum;
  rl_small_t as_small;
  rl_bool_t as_bool;
  rl_nul_t as_nul;
  rl_stream_t as_stream;
  rl_native_t as_native;
  rl_object_t *as_object;
} rl_data_t;

typedef enum
  {
    real_value_type,
    fixnum_value_type,
    small_value_type,
    bool_value_type,
    nul_value_type,
    stream_value_type,
    native_value_type,
    object_value_type
  } value_type_t;

/* globals */
/* tags */
#define QNAN        0x7ff8000000000000ul
#define SIGNBIT     0x8000000000000000ul

/* value tags */
/* immediate tags */
#define REAL        0x0000000000000000ul
#define SMALL       0x7ffc000000000000ul
#define FIXNUM      0x7ffd000000000000ul
#define GLYPH       0x7ffe000000000000ul
#define BOOL        0x7fff000000000000ul
#define NUL         0xfffc000000000000ul
#define STREAM      0xfffd000000000000ul
#define NATIVE      0xfffe000000000000ul
#define OBJECT      0xffff000000000000ul

/* masks */
#define TAGMASK     0xffff000000000000ul
#define PTRMASK     (~TAGMASK)

/* values */
#define TRUE        0x7ffc000000000001ul
#define FALSE       0x7ffc000000000000ul

/* API */
rl_type_t *rl_typeof( rl_value_t x );
bool rl_isa( rl_value_t x, rl_type_t *type );
void mark_value( rl_value_t x );
void *toptr( rl_value_t x );

/* runtime */
void rl_vm_value_init( void );
void rl_vm_value_mark( void );
void rl_vm_value_cleanup( void );

/* convenience */
#define as_value( x ) (((rl_data_t)(x)).as_value)

#define tagof( x ) ((x)&TAGMASK)
#define dataof( x ) ((x)&PTRMASK)
#define tag( x, t ) (dataof(as_value(x))|(t))

#endif
