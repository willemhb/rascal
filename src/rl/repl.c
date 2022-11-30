
#include "vm/error.h"

#include "obj/stream.h"

#include "rl/eval.h"
#include "rl/read.h"
#include "rl/prin.h"
#include "rl/repl.h"

#define PROMPT ">>> "

/* commentary */

/* C types */

/* globals */

/* API */
void repl(void)
{
  value_t x, v;
  
  for (;;)
    {
      fprintf(Outs, PROMPT);

      x = readln(Ins);

      if ( recover() )
	continue;

      v = eval(x);

      if ( recover() )
	continue;

      prinln(Outs, v);
    }
}

/* runtime */
void rl_rl_repl_init( void ) {}
void rl_rl_repl_mark( void ) {}

