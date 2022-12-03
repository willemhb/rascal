#ifndef rl_obj_glyph_h
#define rl_obj_glyph_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */
extern datatype_t GlyphType;

/* API */
value_t glyph( glyph_t glyph );

/* runtime dispatch */
void rl_obj_glyph_init( void );
void rl_obj_glyph_mark( void );
void rl_obj_glyph_cleanup( void );

/* convenience */

#endif
