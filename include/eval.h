#ifndef rascal_eval_h
#define rascal_eval_h

#include "common.h"

// C types --------------------------------------------------------------------

// utilities ------------------------------------------------------------------
size_t rsp_eval(value_t x, port_t *ios);

// native functions -----------------------------------------------------------
value_t native_eval(value_t *args, size_t n_args);

// initialization -------------------------------------------------------------
void eval_init(void);

#endif
