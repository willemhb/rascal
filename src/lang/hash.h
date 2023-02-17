#ifndef hash_h
#define hash_h

#include "value.h"
#include "utils.h"

/* API */
uhash hash_val(Val x, bool deep_hash);

/* initialization */
void hash_init(void);

#endif
