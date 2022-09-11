#ifndef rascal_runtime_h
#define rascal_runtime_h

#include <setjmp.h>

#include "types.h"

// globals --------------------------------------------------------------------
extern jmp_buf Toplevel;

// forward declarations -------------------------------------------------------
void error(value_t agitant, char *fname, char *message, ...);
void require(bool test, value_t agitant, char *fname, char *message, ...);

#endif
