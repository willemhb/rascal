#ifndef rascal_equal_h
#define rascal_equal_h

#include "common.h"

// external API
bool     rlSame(Value x, Value y);
bool     rlEqual(Value x, Value y);
int      rlOrder(Value x, Value y);
uint64_t rlHash(Value x);

#endif
