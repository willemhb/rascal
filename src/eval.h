#ifndef eval_h
#define eval_h

#include "value.h"

// globals --------------------------------------------------------------------
#define N_STACK UINT16_MAX

extern value_t Stack[N_STACK], *Sp, *Se;
extern value_t Quote;

// API ------------------------------------------------------------------------
// stack ----------------------------------------------------------------------
value_t* peep(int i);
value_t  peek(int i);
value_t* push(value_t x);
value_t  pop(void);
value_t* pushn(usize n);
value_t  popn(usize n);

// interpreter ----------------------------------------------------------------
value_t  eval(value_t x);
value_t  apply(value_t x, list_t* args, bool noev);
value_t  invoke(value_t f, usize n, value_t* args);
void     repl(void);

#endif
