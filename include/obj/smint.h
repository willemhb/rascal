#ifndef rl_obj_smint_h
#define rl_obj_smint_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern type_t SmintType;

/* API */

/* runtime */
void rl_obj_smint_init( void );
void rl_obj_smint_mark( void );

/* convenience */
static inline bool    is_smint( value_t x ) { return get_tag(x) == SMINT; }
static inline smint_t as_smint( value_t x ) { return (smint_t)untag(x); }

#endif
