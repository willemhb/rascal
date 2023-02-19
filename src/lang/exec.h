#ifndef lang_exec_h
#define lang_exec_h

#include "base/object.h"
#include "base/opcode.h"

/* API */
Val exec(Func *chunk);
Val exec_at(Func *chunk, OpCode label, uint16 argx, uint16 argy);

#endif
