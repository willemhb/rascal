#ifndef rascal_base_type_h
#define rascal_base_type_h

#include "rascal.h"

/* C types */
enum RlType {
  RealType,
  FixNumType,
  SmallType,
  UnitType,
  BoolType,
  GlyphType,
  SymbolType,
  ListType,
  PairType,
  TupleType,
  ChunkType,
  ClosureType,
  NativeType,
  UpValueType,
  NoneType,
  AnyType
};

/* API */
RlType  rl_type(Value x);
bool    has_rl_type(Value x, RlType type);
char   *type_name(RlType type);

#endif
