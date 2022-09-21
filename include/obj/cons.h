#ifndef rascal_cons_h
#define rascal_cons_h

#include "obj.h"

typedef struct cons_t
{
  OBJ_HEAD;
  val_t          hd;
  struct cons_t *tl;
} cons_t;



#endif
