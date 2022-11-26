#include <string.h>
#include <assert.h>

#include "obj/array.h"
#include "obj/type.h"

#include "vm/memory.h"

#include "util/number.h"

/* commentary */

/* C types */

/* globals */
const size_t min_cap = 8;

/* API */

array_t make_array( type_t type, size_t n, void *ini )
{
  size_t cap   = pad_array_size(n+type->stringp, 0);
  size_t base  = type->obsize + sizeof(void*);
  size_t data  = cap * type->elsize;
  size_t total = base + data;

  struct array_t *out = alloc(base);

  out->len      = n;
  out->alloc    = cap;
  out->inlinep  = false;
  out->dynamicp = true;
  out->obj.type = type;
  out->obj.size = total;

  void *space   = alloc(data);

  if (ini)
    memcpy(space, ini, n * type->elsize);

  *(void**)(out->obj.space) = space;

  return space;
}

size_t resize_array( array_t array, size_t new_count )
{
  struct array_t *header = array_header(array);
  type_t          type   = header->obj.type;

  assert(header->dynamicp);

  size_t new_cap         = pad_array_size(new_count+type->stringp, header->alloc);

  if ( new_cap != header->alloc )
      *(void**)array = adjust_array(array, header->len, new_count, type->elsize);

  header->len   = new_count;
  header->alloc = new_cap;

  return header->len;
}

void free_array( array_t array )
{
  struct array_t *header = array_header(array);

  if (!header->inlinep)
    {
      array_t data = array_data(array);
      dealloc(data, header->alloc * header->obj.type->elsize);
    }

  dealloc(header, sizeof(struct array_t));
}

/* convenience */
size_t pad_array_size( size_t new_count, size_t old_cap )
{
  size_t new_cap = max(old_cap, min_cap);

  while ( new_count < new_cap/2 )
    new_cap >>= 1;
  
  while ( new_count > new_cap )
    new_cap <<= 1;

  return max(new_cap, min_cap);
}
