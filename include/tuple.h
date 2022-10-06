#ifndef rascal_tuple_h
#define rascal_tuple_h

#include "array.h"

struct tuple_t
{
  obj_t    obj;
  arity_t  len;
  type_t   rtype;
  hash_t   hash;
  val_t    data[0];
};

// globals
extern val_t EmptyTuple;

// forward declarations
tuple_t *new_tuple(arity_t len);
void     init_tuple(tuple_t *tuple, arity_t len, val_t *data);
void     mark_tuple(obj_t *obj);
void     free_tuple(obj_t *obj);

void     tuple_init( void );

#endif
