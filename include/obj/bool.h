#ifndef rl_obj_bool_h
#define rl_obj_bool_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern type_t BoolType;

/* API */
bool is_truthy( value_t x );
bool is_falsey( value_t x );

/* runtime */
void rl_obj_bool_init( void );
void rl_obj_bool_mark( void );

/* convenience */
#define TRUE    0x7ffd000000000001ul
#define FALSE   0x7ffd000000000000ul

static inline bool as_bool( value_t x ) { return x == true; }
static inline bool is_bool( value_t x ) { return get_tag(x) == BOOLEAN; }

#endif
