#ifndef eval_h
#define eval_h

#include "common.h"
#include "object.h"

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t eval( value_t x );
value_t exec( function_t* fn );
void    repl( void );

// misc _______________________________________________________________________
bool is_literal( value_t x );
void vm_init( void );

#endif
