#ifndef rascal_obj_h
#define rascal_obj_h

#include "val.h"

struct obj_t
{
  obj_t      *next;
  val_type_t  type  : 30;
  val_type_t  black :  1;
  val_type_t  gray  :  1;
  arity_t     arity;
};

#define OBJ_HEAD \
  obj_t object

// forward declarations -------------------------------------------------------
void init_obj( obj_t *obj, val_type_t type, flags_t fl );
void mark_obj( obj_t *obj );

#endif
