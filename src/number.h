#ifndef rascal_number_h
#define rascal_number_h

#include "value.h"

// C types --------------------------------------------------------------------

// forward declarations -------------------------------------------------------
bool   equalNumbers( Value x, Value y );
hash_t hashNumber( Value x );
void   printNumber( Value x );

#endif
