#ifndef rascal_obj_array_h
#define rascal_obj_array_h

#include "obj/object.h"

// describe macros
#define DARRAY(V)				\
  OBJECT					\
  size_t  length;				\
  size_t  capacity;				\
  V      *data;

// generic array type
typedef struct
{
  DARRAY(void)
} array_t;

// forward declarations
size_t pad_alist_size( size_t oldl, size_t newl, size_t oldc, size_t newc );

#endif
