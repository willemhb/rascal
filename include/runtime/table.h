#ifndef rascal_runtime_table_h
#define rascal_runtime_table_h

#include "rascal.h"

// exports --------------------------------------------------------------------
value_t intern( root_t *t, char *s, hash_t h, value_t (*do_intern)( value_t* b, char *s, hash_t h ) );

#endif
