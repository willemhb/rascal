#include <assert.h>

#include "vm/error.h"

#include "obj/cons.h"
#include "obj/symbol.h"

#include "rl/stx/if.h"

/* commentary */

/* C types */

/* globals */
/* if symbol */
value_t If;

/* API */
bool is_if_form( cons_t *form )
{
  assert(form);
  return get_cons_car(form) == If;
}

int check_if_syntax( cons_t *form )
{
  int l = cons_len(form);

  if ( l == 3 || l == 4 )
    return l;

  panic("Incorrect arity for 'if'");
  return -1;
}

value_t if_test( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_cadr(form);
}

value_t if_then( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_caddr(form);
}

value_t if_otherwise( cons_t *form, int fl )
{
  if ( fl == 3 )
    return NUL;

  return get_cons_cadddr(form);
}

/* runtime */
void rl_rl_stx_if_init( void ) { If = symbol("if"); }

void rl_rl_stx_if_mark( void ) {}

void rl_rl_stx_if_cleanup( void ) {}
