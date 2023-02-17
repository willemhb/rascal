#ifndef type_h
#define type_h

#include "base/value.h"

/* C types */
typedef enum Type Type;

enum Type {
  REAL_TYPE,
  INT_TYPE,
  UNIT_TYPE,
  BOOL_TYPE,
  GLYPH_TYPE,
  SYM_TYPE,
  STREAM_TYPE,
  FUNC_TYPE,
  CONS_TYPE,
  VEC_TYPE,
  TABLE_TYPE,
  BIN_TYPE
};

/* API */
Type val_type_of(Val v);
Type obj_type_of(Obj* o);
bool val_has_type(Val v, Type t);
bool obj_has_type(Obj* o, Type t);

// generics
#define type_of(x)     GENERIC_2(type_of, x)
#define has_type(x, t) GENERIC_2(has_type, x, t)

#endif
