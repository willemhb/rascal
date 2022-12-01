#ifndef rl_rl_stx_if_h
#define rl_rl_stx_if_h

#include "rascal.h"

/* commentary

   handling 'if' special form. */

/* C types */

/* globals */
/* form name */
extern value_t If;

/* API */
bool    is_if_form( cons_t *form );
int     check_if_syntax( cons_t *form );
value_t if_test( cons_t *form, int fl );
value_t if_then( cons_t *form, int fl );
value_t if_otherwise( cons_t *form, int fl );

/* runtime */
void rl_rl_stx_if_init( void );
void rl_rl_stx_if_mark( void );
void rl_rl_stx_if_cleanup( void );

/* convenience */

#endif
