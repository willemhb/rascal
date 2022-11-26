#ifndef rl_obj_real_h
#define rl_obj_real_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern struct type_t RealType;

/* API */

/* runtime */
void rl_obj_real_init( void );

/* convenience */
#define is_real( x ) (((x)&QNAN) != QNAN)
#define as_real( x ) (((rl_data_t)(x)).as_real)

#endif
