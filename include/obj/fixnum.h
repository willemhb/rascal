#ifndef rl_obj_fixnum_h
#define rl_obj_fixnum_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern type_t FixnumType;

/* API */

/* runtime */
void rl_obj_fixnum_init( void );
void rl_obj_fixnum_mark( void );

/* convenience */
static inline bool     is_fixnum( value_t x ) { return get_tag(x) == FIXNUM; }
static inline fixnum_t as_fixnum( value_t x ) { return untag(x); }

#endif
