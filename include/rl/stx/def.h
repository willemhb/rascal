#ifndef rl_rl_stx_def_h
#define rl_rl_stx_def_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */
extern value_t Def;

/* API */
bool    is_def_form( cons_t *form );
int     check_def_syntax( cons_t *form );
value_t def_name( cons_t *form, int fl );
value_t def_bind( cons_t *form, int fl );

/* runtime dispatch */
void rl_rl_stx_def_init( void );
void rl_rl_stx_def_mark( void );
void rl_rl_stx_def_cleanup( void );

/* convenience */

#endif
