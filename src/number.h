#ifndef rascal_number_h
#define rascal_number_h

#include "value.h"

// C types --------------------------------------------------------------------
typedef enum
  {
   C_SINT8 =1, C_UINT8,  C_LATIN1,  C_ASCII,   C_UTF8,
   C_SINT16,   C_UINT16, C_UTF16,
   C_SINT32,   C_UINT32, C_UTF32,   C_FLOAT32,
   C_SINT64,   C_UINT64, C_POINTER, C_FLOAT64,
  } CType;

// forward declarations -------------------------------------------------------
Bool   equalNumbers( Value x, Value y );
Int    orderNumbers( Value x, Value y );
Hash   hashNumber( Value x );
Void   printNumber( Value x );

// utils ----------------------------------------------------------------------
UInt64 ceilLog2( UInt64 u );

// native functions -----------------------------------------------------------
Value nativeAdd( Value *args, int nArgs );
Value nativeSub( Value *args, int nArgs );
Value nativeMul( Value *args, int nArgs );
Value nativeDiv( Value *args, int nArgs );

// statics & macros -----------------------------------------------------------
#define max(a, b)				\
  ({						\
    typeof(a) _a_ = a;				\
    typeof(b) _b_ = b;				\
    _a_ < _b_ ? _b_ : _a_;			\
  })

#endif
