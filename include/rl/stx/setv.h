#ifndef rl_rl_stx_setv_h
#define rl_rl_stx_setv_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */
extern value_t Setv;

/* API */
bool    is_setv_form( cons_t *form );
int     check_setv_syntax( cons_t *form );
value_t setv_name( cons_t *form, int fl );
value_t setv_bind( cons_t *form, int fl );

/* runtime dispatch */
void rl_rl_stx_setv_init( void );
void rl_rl_stx_setv_mark( void );
void rl_rl_stx_setv_cleanup( void );

/* convenience */

#endif
