#ifndef rascal_number_h
#define rascal_number_h

#include "value.h"

// forward declarations
Value rlFixnum( Fixnum x );
Value rlReal( Real x );
Value rlAscii( Ascii x );
Value rlPointer( Pointer p );

// utilities
ULong hashULong( ULong x );

// globals
typedef struct Type Type;

extern Type RealType, NulType, FixnumType, PointerType, AsciiType;

extern Type* ImmediateTypes[];

// convenience
#define min( x, y )							\
  ({									\
    typeof(x) _x_ = x;							\
    typeof(y) _y_ = y;							\
    ( _x_ > _y_ ? _y_ : _x_ );						\
  })

#define max( x, y )							\
  ({									\
    typeof(x) _x_ = x;							\
    typeof(y) _y_ = y;							\
    ( _x_ < _y_ ? _y_ : _x_ );						\
  })


#endif
