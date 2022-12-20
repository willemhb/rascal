#ifndef rl_interp_h
#define rl_interp_h

#include "rascal.h"

/* globals */
extern void (*Prin[num_types])(val_t x);
extern char *TypeName[num_types];

/* API */
val_t read(void);
val_t eval(val_t x);
void  prin(val_t x);
void  repl(void);

/* initialization */
void interp_init( void );

#endif
