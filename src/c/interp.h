#ifndef interp_h
#define interp_h

#include "common.h"

#include "object.h"
#include "runtime.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// special forms --------------------------------------------------------------
extern value_t Quote, Do, If, Lmb, Def, Put, Ccc;

// other syntactic markers ----------------------------------------------------
extern value_t Ampersand;

// special constants ----------------------------------------------------------
extern value_t True, False;

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x );
void repl( void );
chunk_t* compile( value_t src );
value_t exec( chunk_t* code );
void toplevel_define( const char* fname, value_t val );
void toplevel_init_interp( void );

#endif
