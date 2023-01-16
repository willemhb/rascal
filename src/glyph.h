#ifndef rl_glyph_h
#define rl_glyph_h

#include "val.h"

/* globals */
extern struct type_t GlyphType;

/* api */
int   stog(char *s);
char* gtos(glyph_t g);

/* convenience */
#define is_glyph(x) has_type(x, &GlyphType)
#define as_glyph(x) (((rl_data_t)(x)).as_glyph)

#endif
