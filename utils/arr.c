#include "utils/arr.h"
#include "mem.h"


size_t pad_alist_size(size_t oldl, size_t newl, size_t oldc)
{
  if (oldc >= newl && newl >= (oldc>>1))			
    return oldc;							
  arity_t newc = ((size_t)newl+(newl>>3)+6)&~(size_t)3;	
  if (newl - oldl > newc - oldc)				
    newc = ((size_t)newl+3)&~(size_t)3;			       
  return newc;
}


size_t pad_stack_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  size_t newc = oldc;
  if (newc < minc)
    newc = minc;
  if (newl > oldl)						
    while (newl > newc)				       
      newc <<= 1;
  else if (oldl > newl)
    while (newc > minc && newl < (newc >> 1))
      newc >>= 1;
  return newc;
}


void resize_array( array_t *array, size_t newl, size_t elsize )
{
  size_t oldc = array->cap,
    oldl = array->len,
    newc = pad_stack_size( oldl, newl, oldc, 8 );

  if (newc != oldc)
    array->data = resize( array->data, oldc*elsize, newc*elsize );

  array->len = newl;
}
