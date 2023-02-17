#ifndef util_io_h
#define util_io_h

#include <stdio.h>

#include "common.h"

/* API */
// misc io --------------------------------------------------------------------
void newln(void);
void fnewln(FILE *ios);
int peekc(void);
int fpeekc(FILE *ios);

#endif
