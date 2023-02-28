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

// signal syntax errors -------------------------------------------------------
value_t read_error(value_t expr);

// interpreter ----------------------------------------------------------------
value_t read(void);
void    print(value_t val);

// initialization -------------------------------------------------------------
void reader_init(void);

#endif
