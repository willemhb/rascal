#ifndef rascal_prin_h
#define rascal_prin_h

#include "port.h"

// globals
extern printer_fn_t Print[N_TYPES];

// forward declarations
val_t     lisp_prin(port_t *port, val_t val);

#endif
