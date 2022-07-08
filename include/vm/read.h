#ifndef rascal_read_h
#define rascal_read_h

#include "rascal.h"

// exports --------------------------------------------------------------------
value_t rsC_read(FILE *ios);
value_t rsC_load(char_t *fname);

#endif
