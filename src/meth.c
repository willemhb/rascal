#include "meth.h"
#include "memory.h"


static inline ord_t order_methods( meth_t *mx, meth_t *my )
{
  if (my == NULL)
    return 0 - (mx != NULL);

  if (mx == NULL)
    return 1;

  
}

meth_t *new_meth( arity_t argc )
{
  return alloc( sizeof(meth_t) + argc * sizeof(type_t) );
}


void init_meth( meth_t *meth, flags_t fl, meth_t *meths, invoke_t invoke, arity_t argc, type_t *argtypes)
{
  init_obj( &meth->obj, METH, fl );
  
}
