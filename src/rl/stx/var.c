#include "vm/error.h"

#include "obj/cons.h"
#include "obj/symbol.h"

#include "rl/stx/var.h"

/* commentary */

/* C types */

/* globals */
value_t Var;

/* API */
bool is_var_form( cons_t *form )
{
  return get_cons_car(form) == Var;
}

int check_var_syntax( cons_t *form )
{
  switch (cons_len(form))
    {
    case 1:
    case 2:
      return panic("Not enough expressions in 'var'");
      
    case 3:
      if ( !is_symbol(get_cons_cadr(form)) )
	return panic("Name is not a symbol in 'var'");

      return 3;

    default:
      return panic("Too many expressions in 'var'");
    }
}

value_t var_name( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_cadr(form);
}

value_t var_bind( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_caddr(form);
}

/* runtime dispatch */
void rl_rl_stx_var_init( void ) { Var = symbol("var"); }
void rl_rl_stx_var_mark( void ) {}
void rl_rl_stx_var_cleanup( void ) {}

