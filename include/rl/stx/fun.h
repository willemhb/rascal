#ifndef rl_rl_stx_fun_h
#define rl_rl_stx_fun_h

#include "rascal.h"

/* commentary */

/* C types */
typedef enum fun_stx_fl_t
  {
    fun_stx_fl_lambda,
    fun_stx_fl_defun
  } fun_stx_fl_t;

/* globals */
extern value_t Fun;

/* API */
bool is_fun_form( cons_t *form );

int     check_fun_syntax( cons_t *form );
value_t fun_name( cons_t *form, int fl );
value_t fun_args( cons_t *form, int fl );
value_t fun_body( cons_t *form, int fl );

/* runtime dispatch */
void rl_rl_stx_fun_init( void );
void rl_rl_stx_fun_mark( void );
void rl_rl_stx_fun_cleanup( void );

/* convenience */

#endif
