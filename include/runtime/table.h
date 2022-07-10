#ifndef rascal_runtime_table_h
#define rascal_runtime_table_h

#include "rascal.h"

// exports --------------------------------------------------------------------
value_t intern( root_t *t, char *s, int n, hash_t h, value_t (*do_intern)( char *s, int n, hash_t h ) );

value_t do_intern_symbol( char *s, int n, hash_t h );
value_t do_intern_character( char *s, int n, hash_t h );

#endif
