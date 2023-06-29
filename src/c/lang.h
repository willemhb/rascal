#ifndef lang_h
#define lang_h

#include "common.h"

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t read( value_t src );
value_t eval( value_t x );
value_t print( FILE* ios, value_t x );

value_t compile( value_t src );
value_t exec( value_t ch );
value_t apply( value_t f, value_t a );

value_t hash( value_t x );
value_t compare( value_t x, value_t y );

// initialization -------------------------------------------------------------
void toplevel_init_lang( void );

#endif
