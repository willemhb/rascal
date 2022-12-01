#ifndef rl_rl_stx_quote_h
#define rl_rl_stx_quote_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */
/* quote symbol */
extern value_t Quote;

/* API */
bool     is_quote_form( cons_t *form );
int      check_quote_syntax( cons_t *form );
value_t  quote_quoted( cons_t *form, int fl );

/* runtime */
void rl_rl_stx_quote_init( void );
void rl_rl_stx_quote_mark( void );
void rl_rl_stx_quote_cleanup( void );

/* convenience */

#endif
