#include <string.h>
#include <assert.h>

#include "obj/array.h"
#include "obj/type.h"

#include "vm/memory.h"

#include "util/number.h"

/* commentary */

/* C types */

/* globals */

/* API */
array_t make_array( type_t type, size_t len, size_t cap, void *ini )
{
  assert(cap >= len);

  type_t eltype  = type->signature[ELTYPE];
  size_t elsize  = type_vmtype_spec(eltype).size;
  size_t arrsize = cap * elsize;
  size_t total   = type->obsize + arrsize;

  struct array_t *out = alloc(total);

  out->len      = len;
  out->cap      = cap;
  out->obj.type = type;
  out->obj.size = total;

  if (ini)
    memcpy(out->space, ini, arrsize);

  return out->space;
}

array_t resize_array( array_t array, size_t new_cap )
{
  size_t old_cap = array_cap(array);
  size_t elsize  = array_elsize(array);

  struct array_t *adjusted = adjust( array_header(array), old_cap * elsize, new_cap * elsize);

  adjusted->cap = new_cap;

  if ( new_cap < adjusted->len )
    adjusted->len = new_cap;

  return adjusted->space;
}

void free_array( array_t array )
{
  dealloc(array_header(array), obj_size(array));
}

/* runtime */

/* convenience */
type_t array_eltype( array_t array )
{
  return obj_type(array)->signature[ELTYPE];
}

size_t array_elsize( array_t array )
{
  return type_vmtype_spec(array_eltype(array)).size;
}

bool array_stringp( array_t array )
{
  return obj_type(array)->stringp;
}
