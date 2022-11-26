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
  object_t    *as_object;
  value_t     *as_values;
  object_t   **as_objects;
} rl_data_t;

/* globals */
/* tags */
#define QNAN        0x7ff8000000000000ul
#define SIGNBIT     0x8000000000000000ul

/* value tags (represent complete rascal values) */
#define REAL        0x0000000000000000ul
#define FIXNUM      0x7ffc000000000000ul
#define SMALL       0x7ffd000000000000ul
#define IMMEDIATE   0x7ffe000000000000ul
#define OBJECT      0x7fff000000000000ul

/* other immediate tags */
#define NUL         0x7ffe000000000000ul
#define GLYPH       0x7ffe000100000000ul
#define BOOLEAN     0x7ffe000200000000ul

/* internal tags (indicate GC/size information, &c) */
#define POINTER     0xfffc000000000000ul // any pointer
#define STRING      0xfffd000000000000ul // pointer to C string
#define VALUES      0xfffe000000000000ul // pointer to array of tagged values
#define OBJECTS     0xffff000000000000ul // pointer to array of untagged object pointers

/* masks */
#define TAGMASK     0xffff000000000000ul
#define PTRMASK     (~TAGMASK)

/* API */
type_t *rl_typeof( value_t x );

/* convenience */
#define as_value( x )   (((rl_data_t)(x)).as_value)
#define get_tag( x )    (as_value(x)&TAGMASK)
#define untag( x )      (as_value(x)&PTRMASK)
#define set_tag( x, t ) (untag(x)|(t))

#define as_pointer( x )					\
  ((void*)_Generic((x),					\
		   value_t:(untag((value_t)(x))),	\
		   default:((typeof(x))(x))))



#endif
