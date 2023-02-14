#ifndef value_h
#define value_h

#include "common.h"

/* C types */
typedef uintptr_t  Val;
typedef double     Real;
typedef int        Small;
typedef bool       Bool;
typedef char       Glyph;
typedef struct Obj Obj;

typedef enum  ValType ValType;
typedef union ValData ValData;

enum ValType {
  REAL_VAL,
  SMALL_VAL,
  UNIT_VAL,
  BOOL_VAL,
  GLYPH_VAL,
  OBJ_VAL
};

union ValData {
  Val    as_val;
  Real   as_real;
  Small  as_small;
  Bool   as_bool;
  Glyph  as_glyph;
  Obj   *as_obj;
};

/* globals */
#define QNAN      0x7ff8000000000000ul
#define SIGN      0x8000000000000000ul

#define SMALL_TAG 0x7ffc000000000000ul
#define UNIT_TAG  0x7ffd000000000000ul
#define BOOL_TAG  0x7ffe000000000000ul
#define GLYPH_TAG 0x7fff000000000000ul
#define OBJ_TAG   0xfffc000000000000ul

#define TAG_MASK  0xffff000000000000ul
#define VAL_MASK  0x0000fffffffffffful

#define NIL_VAL    UNIT_TAG
#define TRUE_VAL  (BOOL_TAG|1)
#define FALSE_VAL (BOOL_TAG|0)

/* convenience macros */
#define TAG_BITS(value) ((value) & TAG_MASK)
#define VAL_BITS(value) ((value) & VAL_MASK)

#define IS_REAL(value)  (((value) & QNAN) != QNAN)
#define IS_SMALL(value) (TAG_BITS(value) == SMALL_TAG)
#define IS_NIL(value)   ((value) == NIL_VAL)
#define IS_BOOL(value)  (((value)|1) == TRUE_VAL)
#define IS_GLYPH(value) (TAG_BITS(value) == GLYPH_TAG)
#define IS_OBJ(value)   (TAG_BITS(value) == OBJ_TAG)

#define AS_REAL(value)  (((ValData)(value)).as_real)
#define AS_SMALL(value) (((ValData)(value)).as_small)
#define AS_NIL(value)   (value)
#define AS_BOOL(value)  ((value) == TRUE_VAL)
#define AS_GLYPH(value) (((ValData)(value)).as_glyph)
#define AS_OBJ(value)   ((Obj*)VAL_BITS(value))

/* API */
ValType val_type(Val x);

Val     mk_glyph(Glyph g);
Val     mk_bool(Bool b);
Val     mk_small(Small s);
Val     mk_real(Real r);
Val     mk_obj(Obj *o);

#endif
