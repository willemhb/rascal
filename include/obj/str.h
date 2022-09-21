#ifndef rascal_str_h
#define rascal_str_h

#include "obj.h"
#include "template/array.h"

typedef struct str_t str_t;

STRING(str, char_t);

// toplevel dispatch ----------------------------------------------------------
void str_mark( void );
void str_init( void );

#endif
