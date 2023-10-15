#ifndef rascal_equal_h
#define rascal_equal_h

#include "common.h"

// external API
bool   sameVal(Value x, Value y);
bool   sameObj(void* x, void* y);
bool   equalVal(Value x, Value y);
bool   equalObj(void* x, void* y);
bool   orderVal(Value x, Value y);
bool   orderObj(void* x, void* y);
hash_t hashVal(Value x);
hash_t hashObj(void* x);

#define same(x, y)  generic2(same, x, x, y)
#define equal(x, y) generic2(equal, x, x, y)
#define order(x, y) generic2(order, x, x, y)
#define hash(x)     generic2(hash, x, x)

#endif
