#ifndef apply_h
#define apply_h

#include "value.h"

// API ------------------------------------------------------------------------
Val     apply(Val func, Val args);
Val     invoke(Func* func, usize n, Val* args);
Method* dispatch(Func* func, usize n, Val* args);

#endif
