#ifndef rascal_apply_h
#define rascal_apply_h

#include "common.h"

// C types --------------------------------------------------------------------

// utilities ------------------------------------------------------------------
value_t rsp_apply(value_t f, value_t args, size_t n_args);

// native functions -----------------------------------------------------------
value_t native_apply(value_t *args, size_t n_args);

// initialization -------------------------------------------------------------
void apply_init(void);

#endif
