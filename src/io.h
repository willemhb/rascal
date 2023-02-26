#ifndef io_h
#define io_h

#include <stdio.h>

#include "common.h"
#include "value.h"

// API ------------------------------------------------------------------------
// utilities ------------------------------------------------------------------
int   fnewln(FILE* ios);
int   newln(void);
int   fpeekc(FILE* ios);
int   peekc(void);

// interpreter ----------------------------------------------------------------
value_t read(void);
void    print(value_t val);

// initialization -------------------------------------------------------------

#endif
