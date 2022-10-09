#ifndef rascal_number_h
#define rascal_number_h

#include "object.h"

// C types
typedef struct
{
  object_t object;
  void    *pointer;
} rsp_ptr_t;

// convenience
static inline bool is_real(value_t value)
{
  return value == QNAN || (value&QNAN) != QNAN;
}

#define is_chr(val)      (((val)&SMASK)==CHAR_TAG)
#define is_int(val)      (((val)&SMASK)==INT_TAG)
#define is_bool(val)     (((val)&SMASK)==BOOL_TAG)

#define as_real(val)     (((val_data_t)(val)).as_real)
#define as_chr(val)      ((char_t)((((val_data_t)(val)).as_val)&(value_t)UINT32_MAX))
#define as_bool(val)     ((val)==TRUE_VAL)

#define NINTFILL 0xffffff0000000000ul
#define INTSIGN  0x0000008000000000ul

static inline int_t as_int(value_t value)
{
  return (int_t)((value&IMASK)|(NINTFILL*!!(value&INTSIGN)));
}

// globals
extern type_t *RealType, *IntType, *Int64Type, *PointerType, *CTypeType, *BoolType;

// arithmetic functions
value_t native_add(value_t *args, arity_t n);
value_t native_sub(value_t *args, arity_t n);
value_t native_mul(value_t *args, arity_t n);
value_t native_div(value_t *args, arity_t n);
value_t native_rem(value_t *args, arity_t n);

#endif
