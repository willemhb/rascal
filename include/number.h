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

// convenience & utilities
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
