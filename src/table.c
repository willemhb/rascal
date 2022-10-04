#include "table.h"

// general table utilities
static const double table_resize_pressure = 0.75;

size_t pad_table_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  size_t newc = oldc;

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
