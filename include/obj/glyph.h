#ifndef rl_obj_glyph_h
#define rl_obj_glyph_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern type_t GlyphType;

/* API */

/* runtime */
void rl_obj_glyph_init( void );
void rl_obj_glyph_mark( void );

/* convenience */
static inline bool    is_glyph( value_t x ) { return get_tag(x) == GLYPH; }
static inline glyph_t as_glyph( value_t x ) { return (glyph_t)untag(x); }

#endif
