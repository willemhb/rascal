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
  toplevel_init_data_type_none();
  toplevel_init_data_type_nul();
  toplevel_init_data_type_eos();
  toplevel_init_data_type_bool();
  toplevel_init_data_type_glyph();
  toplevel_init_data_type_chunk();
  toplevel_init_data_type_alist();
  toplevel_init_data_type_buf16();
  toplevel_init_data_type_ref();
  toplevel_init_data_type_upv();
  toplevel_init_data_type_env();
  toplevel_init_data_type_port();
  toplevel_init_data_type_fun();
  toplevel_init_data_type_sym();
  toplevel_init_data_type_str();
  toplevel_init_data_type_list();
  toplevel_init_data_type_num();
}
