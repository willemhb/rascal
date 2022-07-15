#ifndef rascal_stack_h
#define rascal_stack_h

#include "rascal.h"
// exports --------------------------------------------------------------------
// stack manipulation
index_t push(value_t x);
index_t pushn(int n, ...);

value_t pop(void);
value_t popn(int n);

int rotate(void);
int rotaten(int n);

// helpers
int save(value_t x);
int saven(int n, ...);
void restore(value_t *loc);
void restoren(int n, ...);

#endif
