#ifndef rascal_value_h
#define rascal_value_h

#include "repr.h"

// utilities and typedefs for tagged and generic values
// all possible layouts of the data part of the value
typedef union
{
  value_t     as_val;
  data_t      as_data;

  object_t   *as_obj;
  boxed_t    *as_box;
  record_t   *as_record;
  table_t    *as_table;
  array_t    *as_array;
  hamt_t     *as_hamt;
  amt_t      *as_amt;
  cvalue_t   *as_cvalue;
  type_t     *as_type;

  symbol_t   *as_symbol;
  cons_t     *as_cons;
  pair_t     *as_pair;
  function_t *as_function;
  stream_t   *as_stream;
  control_t  *as_control;
  
  variable_t *as_variable;
  slot_t     *as_slot;
  upvalue_t  *as_upvalue;

  nul_t       as_nul;
  boolean_t   as_boolean;
  repr_t      as_repr;
  Ctype_t     as_ctype;
  opcode_t    as_opcode;
  primitive_t as_primitive;

  ascii_t     as_ascii;
  latin1_t    as_latin1;
  utf8_t      as_utf8;
  utf16_t     as_utf16;
  utf32_t     as_utf32;

  sint8_t     as_sint8;
  uint8_t     as_uint8;
  sint16_t    as_sint16;
  uint16_t    as_uint16;
  sint32_t    as_sint32;
  uint32_t    as_uint32;
  real32_t    as_real32;

  fixnum_t    as_fixnum;
  real_t      as_real;

  pointer_t  *as_pointer;
} rl_value_t;

// tags
#define QNAN_BITS 32764
#define SIGN_BIT  32768

#define QNAN      ((value_t)QNAN_BITS<<48)
#define SIGN      ((value_t)SIGN_BIT<<48)

#define IMM_TAG   ((value_t)(QNAN_BITS|0)<<48)
#define PTR_TAG   ((value_t)(QNAN_BITS|1)<<48)
#define FIX_TAG   ((value_t)(QNAN_BITS|2)<<48)
#define OBJ_TAG   ((value_t)(QNAN_BITS|3)<<48)

#define HDR_TAG   (QNAN|SIGN)

#define NUL_VAL      (IMM_TAG|((value_t)NUL<<32))
#define EOS_VAL      (IMM_TAG|((value_t)CHARACTER<<32)|(value_t)EOF)
#define TRUE_VAL     (IMM_TAG|((value_t)BOOLEAN<<32)|(value_t)1)
#define FALSE_VAL    (IMM_TAG|((value_t)BOOLEAN<<32)|(value_t)0)

#define VT_MASK       (OBJ_TAG|SIGN)
#define IT_MASK       ((value_t)UINT32_MAX<<32)
#define HT_MASK       15
#define IMM_MASK      (UINT32_MAX)
#define PTR_MASK      (~VT_MASK)
#define NXT_MASK      (PTR_MASK&(~((value_t)HT_MASK)))

// forward declarations
repr_t     val_repr( value_t val );
type_t    *val_type( value_t val );
size_t     val_size( value_t val );
size_t     val_unbox( value_t val, size_t size, void *spc );
rl_value_t val_unwrap( value_t val );

// convenience
#define as_ptr(x)							   \
  _Generic((x),								   \
	   value_t:(void*)(((rl_value_t)((value_t)(x))).as_data&PTR_MASK), \
	   default:(typeof((x)))(x))

#define imm_repr(x)  ((x)>>32&255)
#define imm_data(x)  ((x)&IT_MASK)
#define ptr_repr(x)  POINTER
#define ptr_data(x)  ((x)&PTR_MASK)
#define fix_repr(x)  FIXNUM
#define fix_data(x)  ((x)&PTR_MASK)
#define real_repr(x) REAL
#define real_data(x) (x)
#define ob_repr(x)   (*(value_t*)as_ptr(x)&HT_MASK)
#define ob_data(x)   ((x)&PTR_MASK)
#define hdr_repr(x)  ((x)&HT_MASK)


static inline bool is_imm( value_t val )  { return (val&VT_MASK) == IMM_TAG; }
static inline bool is_fix( value_t val )  { return (val&VT_MASK) == FIX_TAG; }
static inline bool is_ptr( value_t val )  { return (val&VT_MASK) == PTR_TAG; }
static inline bool is_hdr( value_t val )  { return (val&VT_MASK) == HDR_TAG; }
static inline bool is_obj( value_t val )  { return (val&VT_MASK) == OBJ_TAG; }
static inline bool is_real( value_t val ) { return (val&QNAN) != QNAN; } 

static inline value_t rl_wrap( rl_value_t value, repr_t r )
{
  if (r < NUL)
    {
      if (!r)
	return NUL_VAL;
      
      return value.as_data|OBJ_TAG;
    }

  if (r < FIXNUM)
    return value.as_data|IMM_TAG|((value_t)r<<32);

  if (r == FIXNUM)
    return value.as_data|FIX_TAG;

  if (r == POINTER)
    return value.as_data|PTR_TAG;

  return value.as_data;
}

#endif
