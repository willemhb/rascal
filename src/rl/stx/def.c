#include "vm/error.h"

#include "obj/symbol.h"
#include "obj/cons.h"

#include "rl/stx/def.h"

/* commentary

   variable defining form. */

/* C types */

/* globals */
value_t Def;

/* API */
bool is_def_form( cons_t *form )
{
  return get_cons_car(form) == Def;
}

int check_def_syntax( cons_t *form )
{
  int len = cons_len(form);

  switch (len)
    {
    case 1:
    case 2:
      return panic("Not enough expressions in 'def'");

    case 3:
      if ( !is_symbol(get_cons_cadr(form)) )
	return panic("Cannot define non-symbol in 'def'");

      return 3;

    default:
      return panic("Too many expressions in 'def'");
    }
}

value_t def_name( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_cadr(form);
}

value_t def_bind( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_caddr(form);
}

/* runtime dispatch */
void rl_rl_stx_def_init( void ) { Def = symbol("def"); }
void rl_rl_stx_def_mark( void ) {}
void rl_rl_stx_def_cleanup( void ) {}

