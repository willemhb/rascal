#ifndef rascal_rlio_h
#define rascal_rlio_h

#include "object.h"

// forward declarations
Void rlPrint( Port *ios, Value x );

// initialization
Void rlioInit( Void );

// globals
extern Port ins, outs, errs;

#endif
