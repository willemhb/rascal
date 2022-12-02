#ifndef rl_rl_stx_val_h
#define rl_rl_stx_val_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */
extern value_t Val;

/* API */
bool    is_val_form( cons_t *form );
int     check_val_syntax( cons_t *form );
value_t val_name( cons_t *form, int fl );
value_t val_bind( cons_t *form, int fl );

/* runtime dispatch */
void rl_rl_stx_val_init( void );
void rl_rl_stx_val_mark( void );
void rl_rl_stx_val_cleanup( void );

/* convenience */

#endif
