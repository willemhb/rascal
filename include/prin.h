#ifndef rascal_prin_h
#define rascal_prin_h

#include "val.h"

// forward declarations -------------------------------------------------------
val_t prin_obj(obj_t *obj, obj_t *ios);
val_t prin_val(val_t val,  obj_t *ios);

// toplevel dispatch ----------------------------------------------------------
void read_mark( void );
void read_init( void );

#endif
