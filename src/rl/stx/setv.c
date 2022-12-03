#include "vm/error.h"

#include "obj/cons.h"
#include "obj/symbol.h"

#include "rl/stx/setv.h"

/* commentary

   assignment form. */

/* C types */

/* globals */
value_t Setv;

/* API */
bool is_setv_form( cons_t *form )
{
  return get_cons_car(form) == Setv;
}

int check_setv_syntax( cons_t *form )
{
  switch (cons_len(form))
    {
    case 1:
    case 2:
      return panic("Not enough expressions in 'setv'");
      
    case 3:
      if ( !is_symbol(get_cons_cadr(form)) )
	return panic("Name is not a symbol in 'setv'");

      return 3;

    default:
      return panic("Too many expressions in 'setv'");
    }
}

value_t setv_name( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_cadr(form);
}

value_t setv_bind( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_caddr(form);
}

/* runtime dispatch */
void rl_rl_stx_setv_init( void ) { Setv = symbol("setv"); }
void rl_rl_stx_setv_mark( void ) {}
void rl_rl_stx_setv_cleanup( void ) {}

