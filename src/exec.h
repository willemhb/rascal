#ifndef exec_h
#define exec_h

#include "object.h"

/* API */
Val exec(UserFn *chunk);
Val exec_at(UserFn *chunk, ubyte label);

#endif
