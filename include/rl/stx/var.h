#ifndef rl_rl_stx_var_h
#define rl_rl_stx_var_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */
extern value_t Var;

/* API */
bool    is_var_form( cons_t *form );
int     check_var_syntax( cons_t *form );
value_t var_name( cons_t *form, int fl );
value_t var_bind( cons_t *form, int fl );

/* runtime dispatch */
void rl_rl_stx_var_init( void );
void rl_rl_stx_var_mark( void );
void rl_rl_stx_var_cleanup( void );

/* convenience */

#endif
