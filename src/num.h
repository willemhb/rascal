#ifndef rl_num_h
#define rl_num_h

#include "val.h"

/* globals */
extern struct type_t NumType;

/* convenience */
#define is_num(x) has_type(x, &NumType)

#endif
