#include "obj/array.h"

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
