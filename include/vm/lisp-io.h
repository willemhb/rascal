#ifndef rascal_read_h
#define rascal_read_h

#include "rascal.h"

// exports --------------------------------------------------------------------
value_t rascal_read(FILE *ios);
value_t rascal_load(char *fname);
value_t rascal_prin( FILE *ios, value_t x );

#endif
