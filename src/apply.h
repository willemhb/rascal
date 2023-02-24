#ifndef apply_h
#define apply_h

#include "value.h"

/*
 *
 * function calling protocol functions, but also the dispatch mechanism go here.
 * 
 */

// API ------------------------------------------------------------------------
Val     apply(Val func, Val args);
Val     invoke(Func* func, usize n, Val* args);
Func*   add_method(Func* func, bool va, Val method, Tuple* sig);
Method* dispatch(Func* func, usize n, Val* args);

#endif
