#ifndef rl_vm_value_h
#define rl_vm_value_h

#include "rascal.h"

/* commentary

   Basic value API (eg type querying, size querying, etc) */

/* C types */
typedef union rl_data_t rl_data_t;

// union of types a tagged value can be
union rl_data_t
{
  rl_value_t as_value;
  rl_bool_t as_bool;
  rl_glyph_t as_glyph;
  rl_stream_t as_stream;
  rl_object_t *as_object;
  rl_real_t as_real;
};

/* globals */
/* tags */
#define QNAN    0x7ff8000000000000ul
#define SIGNBIT 0x8000000000000000ul

/* value tags */
/* immediate tags */
#define BOOL   0x7ffc000000000000ul
#define GLYPH  0x7ffd000000000000ul
#define STREAM 0x7ffe000000000000ul
#define OBJECT 0x7fff000000000000ul

/* masks */
#define TAGMASK   0xffff000000000000ul
#define PTRMASK   (~TAGMASK)

/* special values */
#define NUL   0x7fff000000000000ul
#define TRUE  0x7ffc000000000001ul
#define FALSE 0x7ffc000000000000ul

/* API */
value_type_t value_type( rl_value_t value );
rl_value_t value_tag( rl_value_t value );
rl_value_t value_data( rl_value_t value );
rl_value_t tag_data( rl_data_t data, value_type_t value_type );
bool has_value_type( rl_value_t value, value_type_t value_type );
void mark_value( rl_value_t value );

/* runtime */
void rl_vm_value_init( void );
void rl_vm_value_mark( void );
void rl_vm_value_cleanup( void );

/* convenience */
#define as_value( x ) (((rl_data_t)(x)).as_value)
#define as_bool( x ) (((rl_data_t)(x)).as_bool)
#define as_glyph( x ) (((rl_data_t)(x)).as_glyph)
#define as_stream( x ) ((rl_stream_t)(as_value(x)&PTRMASK))
#define as_object( x ) ((rl_object_t*)(as_value(x)&PTRMASK))
#define as_real( x ) (((rl_data_t)(x)).as_real)

#endif
