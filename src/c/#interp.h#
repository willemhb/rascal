#ifndef interp_h
#define interp_h

#include "common.h"

#include "object.h"
#include "runtime.h"

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x );
void repl( void );
chunk_t* compile( value_t src );
value_t exec( chunk_t* code );
value_t apply( value_t f, list_t* a );
void toplevel_define( char* name, value_t val );
void toplevel_init_interp( void );

#endif
