#include "obj/alist.h"


size_t pad_array_size( size_t new_count, size_t old_cap )
{
  size_t new_cap = max(old_cap, min_cap);

  while ( new_count < new_cap/2 )
    new_cap >>= 1;
  
  while ( new_count > new_cap )
    new_cap <<= 1;

  return max(new_cap, min_cap);
}
