#ifndef rl_obj_native_h
#define rl_obj_native_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern type_t NativeType;

/* API */

/* initialization */
void rl_obj_native_init( void );
void rl_obj_native_mark( void );

/* convenience */
static inline bool        is_native( value_t x ) { return get_tag(x) == NATIVE; }
static inline native_fn_t as_native( value_t x ) { return (native_fn_t)as_pointer(x); }

#endif
