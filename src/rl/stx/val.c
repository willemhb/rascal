#include "vm/error.h"

#include "obj/cons.h"
#include "obj/symbol.h"

#include "rl/stx/val.h"

/* commentary

   assignment form. */

/* C types */

/* globals */
value_t Val;

/* API */
bool is_val_form( cons_t *form )
{
  return get_cons_car(form) == Val;
}

int check_val_syntax( cons_t *form )
{
  switch (cons_len(form))
    {
    case 1:
    case 2:
      return panic("Not enough expressions in 'val'");
      
    case 3:
      if ( !is_symbol(get_cons_cadr(form)) )
	return panic("Name is not a symbol in 'val'");

      return 3;

    default:
      return panic("Too many expressions in 'val'");
    }
}

value_t val_name( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_cadr(form);
}

value_t val_bind( cons_t *form, int fl )
{
  (void)fl;

  return get_cons_caddr(form);
}

/* runtime dispatch */
void rl_rl_stx_var_init( void ) { Val = symbol("val"); }
void rl_rl_stx_var_mark( void ) {}
void rl_rl_stx_var_cleanup( void ) {}

