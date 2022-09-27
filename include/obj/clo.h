#ifndef rascal_clo_h
#define rascal_clo_h

#include "obj.h"
#include "obj/envt.h"
#include "obj/func.h"

typedef struct clo_t clo_t;

// runtime function representation --------------------------------------------
struct clo_t
{
  OBJ_HEAD

  func_t *func;
  envt_t *envt;
  objs_t *upvals;
  objs_t *toplevel;
};

#endif
