#ifndef rascal_prin_h
#define rascal_prin_h

#include "value.h"

// C types --------------------------------------------------------------------
typedef Void (*PrinValFn)(Value val);
typedef Void (*PrinObjFn)(Obj *obj);

// globals --------------------------------------------------------------------
extern PrinValFn PrinValDispatch[NUM_TYPES];
extern PrinObjFn PrinObjDispatch[NUM_TYPES];

// forward declarations -------------------------------------------------------
#define prinLisp(x)				\
  _Generic((x),					\
	   Value:prinVal,			\
	   Obj*:prinObj)(x)

Void prinVal( Value val );
Void prinObj( Obj *obj );

#endif
