/* Character type. Pretty straightforward, most APIs are elsewhere. */
// headers --------------------------------------------------------------------
#include "data/glyph.h"

#include "lang/io.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void print_glyph(Port* ios, Expr x);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void print_glyph(Port* ios, Expr x) {
  Glyph g = as_glyph(x);

  if ( g < CHAR_MAX && CharNames[g] )
    pprintf(ios, "\\%s", CharNames[g]);

  else
    pprintf(ios, "\\%c", g);
}

// external -------------------------------------------------------------------
Glyph as_glyph(Expr x) {
  return x & XVMSK;
}

Expr tag_glyph(Glyph x) {
  return ((Expr)x) | GLYPH_T;
}

// initialization -------------------------------------------------------------
void toplevel_init_data_glyph(void) {
     Types[EXP_GLYPH] = (ExpTypeInfo) {
    .type     = EXP_GLYPH,
    .name     = "glyph",
    .obsize   = 0,
    .print_fn = print_glyph
     };
}
