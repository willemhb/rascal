#ifndef rl_bool_h
#define rl_bool_h

#include "val.h"

/* globals */
extern struct type_t BoolType;

#define TRUE  (BOOL|0x1ul)
#define FALSE (BOOL|0x0ul)

/* API */
bool Cbool(val_t x);

/* convenience */
#define is_bool(x) has_type(x, &BoolType)
#define as_bool(x) (((rl_data_t)(x)).as_bool)

#endif
