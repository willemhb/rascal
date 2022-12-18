#ifndef rl_vm_val_h
#define rl_vm_val_h

#include "rascal.h"

/* commentary

   Basic value API (eg type querying, size querying, etc) */

/* C types */
typedef union data_t data_t;

// union of types a tagged value can be
union data_t
{
  val_t as_value;
  bool_t as_bool;
  glyph_t as_glyph;
  stream_t as_stream;
  obj_t *as_object;
  real_t as_real;
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

/* API */
val_type_t val_type( val_t val );
val_t val_tag( val_t val );
val_t val_data( val_t val );
val_t tag_data( data_t data, val_type_t val_type );
bool has_val_type( val_t val, val_type_t val_type );
void mark_val( val_t val );

/* runtime */
void rl_vm_val_init( void );
void rl_vm_val_mark( void );
void rl_vm_val_cleanup( void );

/* convenience */
#define as_val( x )    (((data_t)(x)).as_val)
#define as_bool( x )   (((data_t)(x)).as_bool)
#define as_glyph( x )  (((data_t)(x)).as_glyph)
#define as_stream( x ) ((stream_t)(as_val(x)&PTRMASK))
#define as_obj( x )    ((obj_t*)(as_val(x)&PTRMASK))
#define as_real( x )   (((data_t)(x)).as_real)

#endif
