#ifndef rascal_base_value_h
#define rascal_base_value_h

#include "rascal.h"

/* C types */
enum ValType {
  RealVal,
  SmallVal,
  FixNumVal,
  UnitVal,
  BoolVal,
  GlyphVal,
  ObjectVal
};

#include "decl/alist.h"

ALIST(Values, Value);

/* globals (mostly tags) */
#define QNAN      0x7ff8000000000000ul

#define SMALL     0x7ffc000000000000ul
#define FIXNUM    0x7ffd000000000000ul
#define UNIT      0x7ffe000000000000ul
#define BOOL      0x7fff000000000000ul
#define GLYPH     0xfffc000000000000ul
#define OBJECT    0xfffd000000000000ul

/* API */
ValType val_type(Value x);
bool    has_val_type(Value x);

#endif
