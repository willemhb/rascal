#ifndef rascal_lisp_io_h
#define rascal_lisp_io_h

#include "rascal.h"

// utilities ------------------------------------------------------------------
size_t  do_prin( FILE *ios, value_t x );
value_t do_read( FILE *ios );
value_t do_load( char *fname );

#endif
