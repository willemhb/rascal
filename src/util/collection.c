#include "obj/type.h"

#include "vm/memory.h"

#include "util/collection.h"
#include "util/number.h"
#include "util/string.h"

/* commentary */

/* C types */

/* globals */

/* API */

/* convenience */
#define min_cap     8ul
#define load_factor 0.50

size_t pad_buffer_size( size_t new_count, size_t old_cap )
{
  return pad_alist_size(new_count+1, old_cap);
}

size_t pad_alist_size( size_t new_count, size_t old_cap )
{
  size_t new_cap = max(old_cap, min_cap);
  
  if ( new_count > new_cap )
    {
      do
	{
	  new_cap <<= 1;
	} while ( new_count > new_cap );
    }

  else if ( new_count < old_cap/2 && new_cap > min_cap )
    {
      do
	{
	  new_cap >>= 1;
	} while ( new_count < new_cap/2 && new_cap > min_cap );
    }

  return new_cap;
}

size_t pad_py_alist_size( size_t new_count, size_t old_count, size_t old_cap )
{
  if ( new_count < old_cap && new_count > (old_cap>>1))
    return old_cap;

  size_t new_cap = ((size_t)new_count + (new_count>>3) + 6) & ~(size_t)3;

  if ( new_count - old_count > new_cap - new_count )
    new_cap = (new_count + 3) & ~(size_t)3;

  return max(new_cap, min_cap);
}

size_t pad_table_size( size_t new_count, size_t old_cap )
{
  size_t new_cap = old_cap ? : 1;
  
  if ( new_count > 0 && new_count > new_cap * load_factor )
    {
      do
	{
	  new_cap <<= 1;
	} while ( new_count > new_cap * load_factor );
    }

  else if ( new_count > 0 && new_count < new_cap / 2 * load_factor )
    {
      do
	{
	  new_cap >>= 1;
	} while ( new_cap && new_count < new_cap / 2 * load_factor );
    }

  return max(new_cap, min_cap);
}
