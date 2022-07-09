#ifndef rascal_runtime_error_h
#define rascal_runtime_error_h

#include "rascal.h"

// utilities ------------------------------------------------------------------
void rerror( char *fmt, ... );
void require( bool test, char *fmt, ... );

#endif
