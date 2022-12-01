#include "vm/error.h"


#include "obj/symbol.h"
#include "obj/nul.h"
#include "obj/cons.h"

#include "rl/stx/quote.h"

/* commentary */

/* C types */

/* globals */
/* quote symbol */
value_t Quote;

/* API */
bool is_quote_form( cons_t *form )
{
  return get_cons_car(form) == Quote;
}

int check_quote_syntax( cons_t *form )
{
  switch (cons_len(form))
    {
    case 1:  return panic("Not enough expressions in 'quote'");
    case 2:  return 2;
    default: return panic("Too many expressions in 'quote'");
    }
}

value_t quote_quoted( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_cadr(form);
}

/* runtime */
void rl_rl_stx_quote_init( void )
{
  Quote = symbol("quote");
}

void rl_rl_stx_quote_mark( void ) {}
void rl_rl_stx_quote_cleanup( void ) {}

