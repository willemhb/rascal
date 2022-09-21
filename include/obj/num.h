#ifndef rascal_num_h
#define rascal_num_h

#include "val.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    C_type_sint8=1, C_type_uint8,  C_type_ascii,   C_type_latin1, C_type_utf8,
    C_type_sint16,  C_type_uint16, C_type_utf16,
    C_type_sint32,  C_type_uint32, C_type_utf32,   C_type_float32,
    C_type_sint64,  C_type_uint64, C_type_pointer, C_type_float64
  } C_type_t;

// forward declarations & generics --------------------------------------------
size_t   C_type_size( C_type_t Ctype );
uint64_t ceil_log2(uint64_t i);
uint64_t floor_log2( uint64_t i);

// toplevel dispatch ----------------------------------------------------------
void stack_mark( void );
void stack_init( void );

// convenience ----------------------------------------------------------------
#define Ctypeof(type)					\
  _Generic((type)0,					\
	   int8_t:   C_type_sint8,			\
	   uint8_t:  C_type_uint8,			\
	   int16_t:  C_type_sint16,			\
	   uint16_t: C_type_uint16,			\
	   int32_t:  C_type_sint32,			\
	   uint32_t: C_type_uint32,			\
	   float:    C_type_float32,			\
	   int64_t:  C_type_sint64,			\
	   uint64_t: C_type_uint64,			\
	   double:   C_type_float64,			\
	   void*:    C_type_pointer)

static inline size_t size_of_C_type( C_type_t C )
{
  switch(C)
    {
    case C_type_sint8  ... C_type_utf8:    return 1;
    case C_type_sint16 ... C_type_utf16:   return 2;
    case C_type_sint32 ... C_type_float32: return 4;
    default:                               return 8;
    }
}

#define max(a, b)				\
  ({						\
    typeof(a) _a_ = a;				\
    typeof(b) _b_ = b;				\
    _a_ < _b_ ? _b_ : _a_;			\
  })

#endif
