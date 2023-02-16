#ifndef apply_h
#define apply_h

#include "value.h"

/* API */
Val apply(Val func, Val args);
Val invoke(Obj *func, int n_args, Val *args);
Val call(Obj *func, int n_args, Val *args);

// convenience macros ---------------------------------------------------------
#define call0(func) call((func), -1, NULL)
#define call1(func, x)					\
  ({							\
    Val _x = x;						\
    call((func), 1, &x)					\
  })

#define call2(func, x, y)			\
  ({						\
    Val _args[] = { (x), (y) };			\
    call((func), 2, _args);			\
  })

#endif
