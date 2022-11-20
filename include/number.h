#ifndef rascal_number_h
#define rascal_number_h

#include "value.h"

// forward declarations
Value rlFixnum( Fixnum x );
Value rlReal( Real x );

// globals
typedef struct Type Type;

extern Type RealType, NulType, FixnumType;

extern Type* ImmediateTypes[];

#endif
