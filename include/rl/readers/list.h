#ifndef rl_rl_readers_list_h
#define rl_rl_readers_list_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
int read_list( reader_t *reader, int dispatch );
int read_list_rpar_error( reader_t *reader, int dispatch );
int read_list_dot_error( reader_t *reader, int dispatch );

/* runtime */
void rl_rl_readers_list_init( void );
void rl_rl_readers_list_mark( void );
void rl_rl_readers_list_cleanup( void );

#endif
