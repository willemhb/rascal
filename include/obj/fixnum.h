#ifndef rl_obj_fixnum_h
#define rl_obj_fixnum_h

#include "rascal.h"

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern datatype_t FixnumType;

/* API */
/* constructors */
value_t fixnum( fixnum_t x );

/* runtime dispatch */
void rl_obj_fixnum_init( void );
void rl_obj_fixnum_mark( void );
void rl_obj_fixnum_cleanup( void );

/* convenience */
#define is_fixnum( x ) (tagof(x)==FIXNUM)
#define as_fixnum( x ) ((x)&PTRMASK)

#endif
