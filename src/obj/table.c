#include "vm/memory.h"

#include "util/number.h"

#include "tpl/impl/table.h"

/* commentary */

/* C types */

/* globals */
const double table_load_factor = .75;
const size_t table_min_cap = 8;

/* API */
size_t pad_table_size( size_t new_count, size_t old_cap )
{
  size_t new_cap = clog2(max(old_cap, table_min_cap));

  while ( new_count < new_cap*table_load_factor/2 )
    {
      new_cap >>= 1;
    }

  while ( new_count > new_cap*table_load_factor )
    {
      new_cap <<= 1;
    }

  return max(new_cap, table_min_cap);
}
