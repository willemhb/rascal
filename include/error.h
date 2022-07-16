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
type_t argt( const char *fn, value_t got, type_t expect );
type_t oargt( const char *fn, value_t got, int n, ... );
int    argbounds( const char *fn, int got, int max );

value_t listof( value_t x, bool proper, bool (*validate)(value_t x) );
value_t vectorof( value_t x, bool (*validate)(value_t x) );
value_t arrayof( value_t *a, size_t n, bool (*validate)(value_t x) );

#endif
