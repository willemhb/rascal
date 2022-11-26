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
  small_t      as_small;
  glyph_t      as_glyph;

  void        *as_pointer;
  stream_t    *as_stream;
  native_fn_t  as_native_fn;
  object_t    *as_object;
  
  cons_t      *as_cons;
} rl_data_t;

typedef enum vm_type_t
  {
    vm_type_real   = 0,
    vm_type_fixnum = 1,
    vm_type_bool   = 2,
    vm_type_glyph  = 3,
    vm_type_small  = 5,
    vm_type_symbol = 8,
    vm_type_cons   = 9,
    
  } vm_type_t;

/* globals */
/* tags */
#define QNAN        0x7ff8000000000000ul
#define SIGNBIT     0x8000000000000000ul

#define TMASK       0xffff000000000000ul
#define OBTAGMASK   0xffff000000000007ul
#define WTMASK      0xffffffff00000000ul
#define PMASK     (~TMASK)
#define OBPTRMASK   0x0000fffffffffff8ul

/* immediate tags */
#define REAL        0x0000000000000000ul
#define FIXNUM      0x7ffc000000000000ul
#define SMALL       0x7ffd000000000000ul
#define POINTER     0x7ffe000000000000ul
#define IMMEDIATE   0x7fff000000000000ul
#define NUL         0x7fff000000000000ul
#define BOOLEAN     0x7fff000100000000ul
#define GLYPH       0x7fff000200000000ul

/* object tags */
/* user object group */
#define SYMBOL      0xfffc000000000000ul
#define CONS        0xfffc000000000001ul
#define VECTOR      0xfffc000000000002ul
#define STRING      0xfffc000000000003ul
#define BYTES       0xfffc000000000004ul
#define MAP         0xfffc000000000005ul
#define SET         0xfffc000000000006ul
#define BIGINT      0xfffc000000000007ul
#define RECORD      0xfffd000000000000ul

/* IO object group */
#define STREAM      0xfffd000000000001ul
#define TEXTBUFFER  0xfffd000000000002ul
#define BINBUFFER   0xfffd000000000003ul
#define READTABLE   0xfffd000000000004ul
#define READER      0xfffd000000000005ul

/* function object group */
#define FUNCTION    0xfffd000000000006ul
#define METHOD      0xfffd000000000007ul
#define NATIVE      0xfffe000000000000ul
#define LAMBDA      0xfffe000000000001ul
#define SCRIPT      0xfffe000000000002ul
#define CONTROL     0xfffe000000000003ul

/* vm object group */
#define BYTECODE    0xfffe000000000004ul
#define CLOSURE     0xfffe000000000005ul
#define NAMESPACE   0xfffe000000000006ul
#define ENVIRONMENT 0xfffe000000000007ul

/* API */

/* convenience */
#define as_value( x )   (((rl_data_t)(x)).as_value)

#define as_pointer( x )					\
  ((void*)_Generic((x),					\
		   value_t:(untag((value_t)(x))),	\
		   default:((typeof(x))(x))))



static inline uword get_tag( value_t x )
{
  if ( (x&QNAN) != QNAN )
    return REAL;

  if ( x&SIGNBIT )
    return x&OBTAGMASK;

  if ( (x&TMASK) == SMALL )
    return x&WTMASK;

  return x&TMASK;
}

#endif
