#ifndef rl_rl_stx_do_h
#define rl_rl_stx_do_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */
extern value_t Do;

/* API */
bool is_do_form( cons_t *form );

int     check_do_syntax( cons_t *form );
value_t do_sequence( cons_t *form, int fl );

/* runtime dispatch */
void rl_rl_stx_do_init( void );
void rl_rl_stx_do_mark( void );
void rl_rl_stx_do_cleanup( void );

/* convenience */

#endif
