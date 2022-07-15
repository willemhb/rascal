#ifndef rascal_runtime_error_h
#define rascal_runtime_error_h

#include "rascal.h"

// utilities ------------------------------------------------------------------
char  *r_fname( void );
void   r_error( const char *fn, const char *fmt, ... );
void   require( const char *fn, bool test, const char *fmt, ... );
int    argc( const char *fn, int got, int expect );
int    vargc( const char *fn, int got, int expect );
int    oargc( const char *fn, int got, int n, ... );

#endif
