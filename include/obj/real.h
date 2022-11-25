#ifndef rl_obj_real_h
#define rl_obj_real_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern type_t RealType;

/* API */

/* runtime */
void rl_obj_real_init( void );
void rl_obj_real_mark( void );

/* convenience */
static inline bool   is_real( value_t x ) { return (x&QNAN) != QNAN; }
static inline real_t as_real( value_t x ) { return ((rl_data_t)x).as_real; }

#endif
