/* Common initilization. */
// headers --------------------------------------------------------------------
#include "data/data.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------

// initialization -------------------------------------------------------------
void toplevel_init_data(void) {
  /* Initialize all builtin type information (probably needs to be called first). */
  toplevel_init_data_none();
  toplevel_init_data_nul();
  toplevel_init_data_eos();
  toplevel_init_data_bool();
  toplevel_init_data_glyph();
  toplevel_init_data_chunk();
  toplevel_init_data_alist();
  toplevel_init_data_buf16();
  toplevel_init_data_ref();
  toplevel_init_data_upv();
  toplevel_init_data_env();
  toplevel_init_data_port();
  toplevel_init_data_fun();
  toplevel_init_data_sym();
  toplevel_init_data_str();
  toplevel_init_data_list();
  toplevel_init_data_num();
}
