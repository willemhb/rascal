#ifndef rl_obj_real_h
#define rl_obj_real_h

#include "vm/value.h"

/* commentary

   64-bit floating point numbers.

   In the near-term this is the only supported numeric type. */

/* C types */

/* globals */
extern datatype_t RealType;

/* API */

/* runtime */
void rl_obj_real_init( void );
void rl_obj_real_mark( void );

/* convenience */
#define is_real( x ) (((x)&QNAN) != QNAN)
#define as_real( x ) (((rl_data_t)(x)).as_real)

#endif
