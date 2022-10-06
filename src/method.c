#include "method.h"
#include "utils.h"

// types implementation

// method table implementation
static hash_t hash_types( types_t *types )
{
  return types->hash;
}

static ord_t cmp_types( types_t *xt, types_t *yt )
{
  ord_t o = u32cmp( xt->types, yt->types, min(xt->arity, yt->arity ) );

  if (o)
    return o;

  return 0 - (xt->arity < yt->arity) + (xt->arity > yt->arity);
}

OBJ_NEW(metht);
TABLE_INIT(metht, meth, METHT);
TABLE_MARK(metht, meth);
TABLE_FREE(metht, meth);
TABLE_RESIZE(metht, meth, METHT);
TABLE_REHASH(metht, meth);
TABLE_PUT(metht, meth, types_t*, signature, hash_types, cmp_types );
TABLE_GET(metht, meth, types_t*, signature, hash_types, cmp_types );

OBJ_NEW(meth);
