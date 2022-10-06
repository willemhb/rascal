#include <string.h>

#include "tuple.h"
#include "memory.h"


tuple_t *new_tuple(arity_t len)
{
  assert(len > 0);
  return alloc(sizeof(tuple_t) + len * sizeof(val_t));
}

void init_tuple(tuple_t *tuple, arity_t len, val_t *data)
{
  init_obj( &tuple->obj, TUPLE, 0 );

  tuple->len   = len;
  tuple->rtype = TUPLE;
  tuple->hash  = 0;

  if (data)
    memcpy( tuple->data, data, len * sizeof(val_t) );
}

ARRAY_MARK(tuple, val_t);

// initialization
tuple_t EmptyTupleObject;

void tuple_init( void )
{
  EmptyTuple = tag_ptr(&EmptyTupleObject, OBJ);
  init_tuple(&EmptyTupleObject, 0, NULL);
}
