/* simple wrapper around a C file object. */
// headers --------------------------------------------------------------------
#include "data/types/port.h"

#include "lang/io.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void free_port(void* ptr);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void free_port(void* ptr) {
  Port* p = ptr;

  close_port(p);
}

// external -------------------------------------------------------------------
Port* mk_port(FILE* ios) {
  Port* p = mk_obj(EXP_PORT, 0);
  p->ios  = ios;

  return p;
}

// initialization -------------------------------------------------------------
void toplevel_init_data_type_port(void) {
  Types[EXP_PORT] = (ExpTypeInfo) {
    .type     = EXP_PORT,
    .name     = "port",
    .obsize   = sizeof(Port),
    .free_fn  = free_port
  };
}
