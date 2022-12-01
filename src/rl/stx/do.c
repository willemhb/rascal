
#include "vm/error.h"

#include "obj/symbol.h"
#include "obj/cons.h"

#include "rl/stx/do.h"

/* commentary */

/* C types */

/* globals */
value_t Do;

/* API */
bool is_do_form( cons_t *form )
{
  return get_cons_car(form) == Do;
}

int check_do_syntax( cons_t *form )
{
  int out = cons_len(form);

  if (out > 1)
    return out;

  return panic("Not enough expressions in 'do'");
}

value_t do_sequence( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_cdr(form);
}

/* runtime dispatch */
void rl_rl_stx_do_init( void ) { Do = symbol("do"); }
void rl_rl_stx_do_mark( void ) {}
void rl_rl_stx_do_cleanup( void ) {}

