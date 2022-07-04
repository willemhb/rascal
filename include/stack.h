#ifndef rascal_stack_h
#define rascal_stack_h

#include "core.h"

// exports --------------------------------------------------------------------
// stack manipulation
index_t push(value_t x);
index_t pushn(arity_t n, ...);

value_t pop(void);
index_t popn(arity_t n);

// dump manipulation
index_t save(value_t x);
index_t saven(arity_t n, ...);
void    unsave(void);
void    unsaven(arity_t n);
void    restore(void_t *loc);
void    restoren(arity_t n, ...);

#endif
