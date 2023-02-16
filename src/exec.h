#ifndef exec_h
#define exec_h

#include "object.h"
#include "opcode.h"

/* API */
Val exec(UserFn *chunk);
Val exec_at(UserFn *chunk, OpCode label, uint16 argx, uint16 argy);

#endif
