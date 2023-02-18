#ifndef data_glyph_h
#define data_glyph_h

#include "base/value.h"

/* API */
bool  is_glyph(Val x);
Glyph as_glyph(Val x);
Val   mk_glyph(Glyph g);

#endif
