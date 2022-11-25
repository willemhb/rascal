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
  bool_t       as_bool;
  fixnum_t     as_fixnum;
  smint_t      as_smint;
  glyph_t      as_glyph;

  void        *as_pointer;
  stream_t    *as_stream;
  native_fn_t  as_native_fn;
  object_t    *as_object;
  
  cons_t      *as_cons;
} rl_data_t;

/* globals */

/* API */
type_t *rl_typeof( value_t x );
size_t  rl_sizeof( value_t x );

/* utilities & convenience */
#define QNAN    0x7ff8000000000000ul

#define TMASK   0xffff000000000000ul
#define PMASK   (~TMASK)

#define NUL     0x7ffc000000000000ul
#define BOOLEAN 0x7ffd000000000000ul
#define GLYPH   0x7ffe000000000000ul
#define FIXNUM  0x7fff000000000000ul
#define SMINT   0xfffc000000000000ul
#define NATIVE  0xfffd000000000000ul
#define STREAM  0xfffe000000000000ul
#define OBJECT  0xffff000000000000ul

/* convenience & utilities */
#define get_tag( x )    (as_value(x)&TMASK)
#define set_tag( x, t ) (untag(x)|(t))
#define untag( x )      (as_value(x)&PMASK)

#define as_value( x )   (((rl_data_t)(x)).as_value)
#define as_pointer( x ) ((void*)untag(x))

static inline bool rl_isa( value_t x, type_t *type )
{
  return rl_typeof(x) == type;
}

#endif
