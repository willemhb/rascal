#ifndef rascal_prin_h
#define rascal_prin_h

#include "port.h"

// globals
extern printer_fn_t Print[N_TYPES];

// forward declarations
value_t     lisp_prin(stream_t *port, value_t value);

#endif
