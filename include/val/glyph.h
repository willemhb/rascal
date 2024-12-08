#ifndef rl_val_glyph_h
#define rl_val_glyph_h

#include "val/value.h"

/* C types */
/* Globals */
// rascal EOF representation
#define EOS        0xffff0003fffffffful // GLYPH | -1

/* API */
#define is_glyph(x) (wtag_bits(x) == GLYPH)
#define as_glyph(x) ((Glyph)wdata_bits(x))

/* Initialization */

#endif
