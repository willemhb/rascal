#include "exec.h"
#include "read.h"
#include "prin.h"

#define INPROMPT  "<< "
#define OUTPROMPT ">> "

void repl( void )
{
  for (;;)
    {
      port_take(&Ins);
      printf(INPROMPT);
      val_t val = lisp_read(&Ins);
      printf("\n"OUTPROMPT);
      lisp_prin(&Outs, val);
      printf("\n");
    }
}
