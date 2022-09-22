#ifndef rascal_cons_h
#define rascal_cons_h

#include "obj.h"

DECL_OBJ(cons);

struct cons_t
{
  OBJ_HEAD;
  val_t   hd;
  cons_t *tl;
  size_t  len;
};

#endif
