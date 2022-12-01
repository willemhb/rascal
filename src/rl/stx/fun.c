#include "vm/error.h"

#include "obj/cons.h"
#include "obj/symbol.h"

#include "rl/stx/fun.h"

/* commentary */

/* C types */

/* globals */
value_t Fun;

/* API */
bool is_fun_form( cons_t *form )
{
  return form && get_cons_car(form) == Fun;
}

int check_fun_syntax( cons_t *form )
{
  int len = cons_len(form);

  int out = fun_stx_fl_lambda;

  if (len < 3)
      return panic("Not enough inputs to 'fun'");

  value_t x = get_cons_cadr(form);

  if ( is_symbol(x) )
    {
      out = fun_stx_fl_defun;
      if ( len < 4 )
	return panic("Not enough inputs to 'fun'");

      x = get_cons_caddr(form);
    }

  if ( !is_arglist(x) )
    return panic("Malformed argument list in 'fun'");

  return out;
}

value_t fun_name( cons_t *form, int fl )
{
  if ( flagp(fl, fun_stx_fl_defun) )
    return Fun;

  return get_cons_cadr(form);
}

value_t fun_args( cons_t *form, int fl )
{
  if ( flagp(fl, fun_stx_fl_defun) )
    return get_cons_caddr(form);

  return get_cons_cadr(form);
}

value_t fun_body( cons_t *form, int fl )
{
  if ( flagp(fl, fun_stx_fl_defun) )
    return get_cons_cdddr(form);

  return get_cons_cddr(form);
}

/* runtime dispatch */
void rl_rl_stx_fun_init( void ) { Fun = symbol("fun"); }
void rl_rl_stx_fun_mark( void ) {}
void rl_rl_stx_fun_cleanup( void ) {}

