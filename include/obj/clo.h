#ifndef rascal_clo_h
#define rascal_clo_h

#include "obj.h"


DECL_OBJ(clo);

// runtime function representation --------------------------------------------
struct clo_t
{
  OBJ_HEAD;

  obj_t *func;
  obj_t *ns;
  obj_t *upvals;
  obj_t *toplevel;
};

#endif
