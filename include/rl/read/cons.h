#ifndef rl_rl_read_cons_h
#define rl_rl_read_cons_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
void read_list( reader_t *reader, int dispatch );
void read_list_error( reader_t *reader, int dispatch );

/* runtime */
void rl_rl_read_cons_init( void );
void rl_rl_read_cons_mark( void );

#endif
