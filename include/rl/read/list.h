#ifndef rl_rl_read_list_h
#define rl_rl_read_list_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
void read_list( reader_t *reader, int dispatch );
void read_list_rpar_error( reader_t *reader, int dispatch );
void read_list_dot_error( reader_t *reader, int dispatch );

/* runtime */
void rl_rl_read_list_init( void );
void rl_rl_read_list_mark( void );

#endif
