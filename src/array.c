#include <string.h>

#include "array.h"
#include "obj.h"
#include "utils.h"
#include "memory.h"

// general array utilities
size_t pad_alist_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  if (oldc >= newl && newl >= (oldc>>1))			
    return oldc;							
  arity32_t newc = ((size_t)newl+(newl>>3)+6)&~(size_t)3;	
  if (newl - oldl > newc - oldc)				
    newc = ((size_t)newl+3)&~(size_t)3;
  if (newc < minc)
    newc = minc;
  return newc;
}

size_t pad_stack_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  size_t newc = oldc;
  if (newl > oldl)						
    while (newl > newc)				       
      newc <<= 1;
  else if (oldl > newl)
    while (newc > minc && newl < (newc >> 1))
      newc >>= 1;
  if (newc < minc)
    newc = minc;
  return newc;
}

// utility array type implementations
// boxed array types
OBJ_NEW(alist);
ARRAY_INIT(alist, val_t, Ctype_uint64, ALIST);
ARRAY_MARK(alist, val_t);
ARRAY_FREE(alist, val_t);
ARRAY_RESIZE(alist, val_t, ALIST, pad_alist_size);
ARRAY_WRITE(alist, val_t);
ARRAY_PUSH(alist, val_t);
ARRAY_POP(alist, val_t);
ARRAY_CLEAR(alist, val_t, ALIST);

OBJ_NEW(stack);
ARRAY_INIT(stack, val_t, Ctype_uint64, STACK );
ARRAY_MARK(stack, val_t);
ARRAY_FREE(stack, val_t);
ARRAY_RESIZE(stack, val_t, STACK, pad_stack_size);
ARRAY_WRITE(stack, val_t);
ARRAY_PUSH(stack, val_t);
ARRAY_POP(stack, val_t);
ARRAY_CLEAR(stack, val_t, STACK);

// 

// globals & initialization

