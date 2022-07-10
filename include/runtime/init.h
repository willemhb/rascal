#ifndef rascal_runtime_init_h
#define rascal_runtime_init_h

#include "rascal.h"

/**
 *
 * this is just a bunch of uninteresting functions for initializing the runtime.
 *
 */

// exports --------------------------------------------------------------------
void init_memory( void );
void init_tables( void );
void init_dispatch( void );
void init_types( void );
void init_names( void );
void init_builtins( void );
void init_globals( void );

#endif
