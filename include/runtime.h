#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "common.h"

void    error( value_t agitant, const char *fname, const char *fmt, ...);
void    require( bool_t test, value_t agitant, const char *fname, const char *fmt, ...);
void    argco( size_t expect, size_t got, const char *fname );
void    vargco( size_t expect, size_t got, const char *fname );
void    argtype( value_t agitant, const char *fname, size_t expect, ...);

#endif
