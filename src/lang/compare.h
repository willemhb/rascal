#ifndef lang_compare_h
#define lang_compare_h

#include "data/value.h"

// APIS & utilities
#define rl_equal(x, y) generic2(equal, x, y)
#define rl_same(x, y) generic2(same, x, y)

bool val_same(value_t vx, value_t vy);
bool obj_same(void* ox, void* oy);
bool val_equal(value_t vx, value_t vy);
bool obj_equal(void* ox, void* oy);

// globals
extern bool (*Equal[NTYPES])(void* ox, void* oy, int bound, table_t* backrefs);

#endif
