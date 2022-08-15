#ifndef rascal_print_h
#define rascal_print_h

#include "common.h"

// C types --------------------------------------------------------------------

// utilities ------------------------------------------------------------------
size_t rsp_print(value_t x, port_t *ios);

// native functions -----------------------------------------------------------
value_t native_print(value_t *args, size_t n_args);

// initialization -------------------------------------------------------------
void value_init(void);

#endif
