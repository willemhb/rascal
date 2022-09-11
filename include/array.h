#ifndef rascal_array_h
#define rascal_array_h

#include "object.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    C_sint8=1, C_uint8=2, C_ascii=3, C_latin1=4, C_utf8=5,

    C_sint16=6, C_uint16=7, C_utf16=8,

    C_sint32=9, C_uint32=10, C_utf32=11, C_float32=12,

    C_sint64=13, C_uint64=14, C_float64=15
  } Ctype_t;

typedef enum
  {
    vector_fl_envt=1,
  } vector_fl_t;

struct binary_t
{
  HEADER;
  void *data;
  uint len, cap;
};

struct vector_t
{
  HEADER;
  value_t *data;
  uint len, cap;
};

// forward declarations -------------------------------------------------------

// macros & statics -----------------------------------------------------------
#define as_binary(x) asa(binary_t*, x, pval)
#define as_vector(x) asa(vector_t*, x, pval)

#define bin_data(x)  getf(binary_t*, x, data)
#define bin_len(x)   getf(binary_t*, x, len)
#define bin_cap(x)   getf(binary_t*, x, cap)

#define vec_data(x)  getf(vector_t*, x, data)
#define vec_len(x)   getf(vector_t*, x, len)
#define vec_cap(x)   getf(vector_t*, x, cap)

#endif
