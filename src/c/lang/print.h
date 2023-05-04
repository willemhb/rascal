#ifndef lang_print_h
#define lang_print_h

#include "data/value.h"

// APIs & utilities
void rl_print(value_t x);

// globals
extern void (*Print[NTYPES])(value_t x, table_t* backrefs);

#endif
