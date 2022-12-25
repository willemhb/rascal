#ifndef rl_prin_h
#define rl_prin_h

#include "rascal.h"

/* globals */
extern void  (*Prin[num_types])(val_t x);
extern char   *TypeName[num_types];

/* API */
void prin(val_t x);
void repl(void);

#endif
