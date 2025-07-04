#ifndef rl_data_glyph_h
#define rl_data_glyph_h

/* Character type. Pretty straightforward, most APIs are elsewhere. */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Glyph as_glyph(Expr x);
Expr  tag_glyph(Glyph x);

// initialization -------------------------------------------------------------
void toplevel_init_data_glyph(void);

#endif
