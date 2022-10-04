#include "table.h"
#include "memory.h"

// general table utilities
static const double table_resize_pressure = 0.75;

void init_ords(arity_t cap, ords_t *ords, arity_t *osize)
{
  if (cap <= INT8_MAX)
    {
      *(int8_t**)ords = alloc_vec( cap, int8_t );
      *osize          = 1;
    }

  else if (cap <= INT16_MAX)
    {
      *(int16_t**)ords = alloc_vec( cap, int16_t );
      *osize           = 2;
    }

  else
    {
      *(int32_t**)ords = alloc_vec( cap, int32_t );
      *osize           = 4;
    }

  // initialize to -1 of correct width
  memset(*(byte_t**)ords, 255, cap * *osize );
}

void free_ords(ords_t ords, arity_t ocap, arity_t osize)
{
  if (osize == 1)
    dealloc_vec( ords.o8, ocap, int8_t );

  else if (osize == 2)
    dealloc_vec( ords.o16, ocap, int16_t );

  else
    dealloc_vec( ords.o32, ocap, int32_t );
}

bool resize_ords(arity_t oldl, arity_t newl, arity_t *oldc, arity_t minc, ords_t *ords, arity_t *osize)
{
  arity_t newc = pad_table_size(oldl, newl, *oldc, minc);

  if (newc != *oldc)
    {
      free_ords(*ords, *oldc, *osize);
      init_ords(newc, ords, osize);
      return true;
    }

  return false;
}

arity_t pad_table_size(arity_t oldl, arity_t newl, arity_t oldc, arity_t minc)
{
  arity_t newc = oldc;

  if (newl > oldl)						
    while (newl > newc*table_resize_pressure)				       
      newc <<= 1;
  else if (oldl > newl)
    while (newc > minc && newl < table_resize_pressure*(newc >> 1))
      newc >>= 1;
  if (newc < minc)
    newc = minc;
  return newc;
}
