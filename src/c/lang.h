#ifndef lang_h
#define lang_h

#include "common.h"

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t read( port_t ios );
value_t load( const char* filename );
value_t print( port_t ios, value_t x );
bool    same( value_t x, value_t y );
bool    equal( value_t x, value_t y );
int     compare( value_t x, value_t y );
uhash   hash( value_t x );

// initialization -------------------------------------------------------------
void toplevel_init_lang( void );

#endif
