#ifndef interp_h
#define interp_h

#include "common.h"

#include "object.h"
#include "runtime.h"

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t  eval( value_t x, envt_t* envt );
void     repl( void );
chunk_t* compile( value_t src, ns_t* ns );
value_t  exec( chunk_t* code, envt_t* envt );

#endif
