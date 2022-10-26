#ifndef rascal_obj_number_h
#define rascal_obj_number_h

#include "obj/cvalue.h"

// type definitions
typedef struct rl_sint64_t
{
  OBJECT;
  sint64_t data;
} rl_sint64_t;

typedef struct rl_uint64_t
{
  OBJECT;
  uint64_t data;
} rl_uint64_t;

// globals
extern type_t SInt8Type,  UInt8Type;
extern type_t SInt16Type, UInt16Type;
extern type_t SInt32Type, UInt32Type;
extern type_t SInt64Type, UInt64Type;
extern type_t Real32Type, Real64Type;
extern type_t FixnumType;

// various numeric union types
extern type_t SintType, UintType, IntType, RealType, NumberType;

#endif
