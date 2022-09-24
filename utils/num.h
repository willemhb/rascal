#ifndef rascal_utils_num_h
#define rascal_utils_num_h

#include <stdint.h>
#include <stddef.h>

// C types --------------------------------------------------------------------
typedef enum
  {
    Ctype_sint8=1, Ctype_uint8,  Ctype_ascii,   Ctype_latin1, Ctype_utf8,
    Ctype_sint16,  Ctype_uint16, Ctype_utf16,
    Ctype_sint32,  Ctype_uint32, Ctype_utf32,   Ctype_float32,
    Ctype_sint64,  Ctype_uint64, Ctype_pointer, Ctype_float64
  } Ctype_t;

// forward declarations & generics --------------------------------------------
size_t   Ctype_size( Ctype_t Ctype );
uint64_t ceil_log2(uint64_t i);
uint64_t floor_log2( uint64_t i);
int      ord_uint(uint64_t x, uint64_t y);

// convenience ----------------------------------------------------------------
#define Ctypeof(type)					\
  _Generic((type)0,					\
	   int8_t:   Ctype_sint8,			\
	   uint8_t:  Ctype_uint8,			\
	   int16_t:  Ctype_sint16,			\
	   uint16_t: Ctype_uint16,			\
	   int32_t:  Ctype_sint32,			\
	   uint32_t: Ctype_uint32,			\
	   float:    Ctype_float32,			\
	   int64_t:  Ctype_sint64,			\
	   uint64_t: Ctype_uint64,			\
	   double:   Ctype_float64,			\
	   void*:    Ctype_pointer)

static inline size_t sizeof_Ctype( Ctype_t C )
{
  switch(C)
    {
    case Ctype_sint8  ... Ctype_utf8:    return 1;
    case Ctype_sint16 ... Ctype_utf16:   return 2;
    case Ctype_sint32 ... Ctype_float32: return 4;
    default:                             return 8;
    }
}

#define max(a, b)				\
  ({						\
    typeof(a) _a_ = a;				\
    typeof(b) _b_ = b;				\
    _a_ < _b_ ? _b_ : _a_;			\
  })

#define min(a, b)				\
  ({						\
    typeof(a) _a_ = a;				\
    typeof(b) _b_ = b;				\
    _a_ > _b_ ? _b_ : _a_;			\
  })


#endif
