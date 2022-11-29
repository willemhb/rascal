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
value_t add_reals( value_t x, value_t y );
value_t sub_reals( value_t x, value_t y );
value_t mul_reals( value_t x, value_t y );
value_t div_reals( value_t x, value_t y );
value_t eq_reals( value_t x, value_t y );
value_t lt_reals( value_t x, value_t y );
value_t gt_reals( value_t x, value_t y );
value_t geq_reals( value_t x, value_t y );
value_t leq_reals( value_t x, value_t y );
value_t neq_reals( value_t x, value_t y );

/* runtime */
void rl_obj_real_init( void );
void rl_obj_real_mark( void );

/* convenience */
#define is_real( x ) (((x)&QNAN) != QNAN)
#define as_real( x ) (((rl_data_t)(x)).as_real)

#endif
