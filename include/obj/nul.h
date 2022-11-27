#ifndef rl_obj_nul_h
#define rl_obj_nul_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */
extern struct type_t NulType;

/* API */

/* runtime */
void rl_obj_nul_init( void );

/* convenience */
#define is_nul( x ) ((x)==NUL)
#define as_nul( x ) (NULL)

#endif
