#ifndef rascal_cons_h
#define rascal_cons_h

#include "obj.h"

typedef struct cons_t
{
  OBJ_HEAD;
  val_t          hd;
  struct cons_t *tl;
} cons_t;

// forward declarations & generics --------------------------------------------
#define cons(a, b)				\
  _Generic((b),					\
	   cons_t*:cons_2,			\
	   size_t:cons_n)((a), (b))

obj_t *cons_2( val_t  hd, cons_t   *tl );
obj_t *cons_n( val_t *args, size_t  n );

// toplevel dispatch ----------------------------------------------------------
void cons_mark( void );
void cons_init( void );

#endif
